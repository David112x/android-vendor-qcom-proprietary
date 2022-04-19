/**********************************************************************
*  Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "QCameraPPEisCore.h"
#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <utils/Log.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include <android/log.h>
#include <vector>

#ifdef _ANDROID_
#include <utils/Log.h>
#include <cutils/properties.h>
#endif
#include "img_dbg.h"

#define DEFAULT_WIDTH 1920
#define DEFAULT_HEIGHT 1080

const char *vertexShaderSource = "                                             \n\
                                   #version 300 es                             \n\
                                   in vec4 inPosition;                         \n\
                                   in mat3 inTransform;                        \n\
                                   in vec3 inTexCoord;                         \n\
                                   out vec3 texCoord;                          \n\
                                                                               \n\
                                  void main()                                  \n\
                                  {                                            \n\
                                      texCoord =  inTexCoord*inTransform;      \n\
                                      gl_Position = inPosition;                \n\
                                  }";

const char *fragmentShaderSource = "                                                                                      \n\
    #version 300 es                                                                   \n\
    #extension GL_EXT_YUV_target : require                                            \n\
                                       precision highp float;                                                            \n\
                                       uniform highp  __samplerExternal2DY2YEXT tex;                                     \n\
                                       in vec3 texCoord;                                                                 \n\
                                       layout (yuv) out vec4 FragColor;                                                  \n\
                                                                                                                         \n\
                                       void main()                                                                       \n\
                                       {                                                                                 \n\
                                           vec2 _texCoord;                                                               \n\
                                           _texCoord = vec2(texCoord.x/texCoord.z, texCoord.y/texCoord.z);               \n\
                                           FragColor = texture(tex, _texCoord);                                          \n\
                                       }";

const char *fragmentShaderSeparateSource = "                                                                                     \n\
                                               #version 300 es                                                                   \n\
                                               #extension GL_OES_EGL_image_external_essl3 : require                              \n\
                                               precision highp float;                                                            \n\
                                               uniform highp samplerExternalOES tex;                                             \n\
                                               in vec3 texCoord;                                                                 \n\
                                               out vec4 FragColor;                                                               \n\
                                                                                                                                 \n\
                                               void main()                                                                       \n\
                                               {                                                                                 \n\
                                                   vec2 _texCoord;                                                               \n\
                                                   _texCoord = vec2(texCoord.x/texCoord.z, texCoord.y/texCoord.z);               \n\
                                                   FragColor = texture(tex, _texCoord);                                          \n\
                                               }";

static EGLint sDefaultConfigAttribs[] = {
    //EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_NONE
};

EGLConfig GetEglConfig(EGLDisplay display, const EGLint* attrib_list,
  Config* cfg)
{
  EGLConfig config = NULL;

  EGLConfig* configs;
  EGLint numConfigs;
  int i;

  eglChooseConfig(display, attrib_list, NULL, 0, &numConfigs);
  configs = (EGLConfig*)malloc(sizeof(EGLConfig) * numConfigs);
  if (!configs) {
    return 0;
  }

  eglChooseConfig(display, attrib_list, configs, numConfigs, &numConfigs);
  // parse all returned configurations and select the one that we really want
  for ( i=0; i<numConfigs; ++i )
  {
    int r, g, b, a, depth, stencil, sample_buffers, samples;
    eglGetConfigAttrib(display,configs[i],EGL_RED_SIZE,&r);
    eglGetConfigAttrib(display,configs[i],EGL_GREEN_SIZE,&g);
    eglGetConfigAttrib(display,configs[i],EGL_BLUE_SIZE,&b);
    eglGetConfigAttrib(display,configs[i],EGL_ALPHA_SIZE,&a);
    eglGetConfigAttrib(display,configs[i],EGL_DEPTH_SIZE,&depth);
    eglGetConfigAttrib(display,configs[i],EGL_STENCIL_SIZE,&stencil);
    eglGetConfigAttrib(display,configs[i],EGL_SAMPLE_BUFFERS,&sample_buffers);
    eglGetConfigAttrib(display,configs[i],EGL_SAMPLES,&samples);
    if ( r == cfg->rBits && g == cfg->gBits && b == cfg->bBits &&
            (!cfg->aBits || a == cfg->aBits) && depth == cfg->depthBits &&
            (!cfg->stencilBits || stencil == cfg->stencilBits) &&
             sample_buffers == cfg->sampleBuffers && samples == cfg->samples )
    {
      config = configs[i];
      break;
    }
  }

  free(configs);

  return config;
}



bool CreateShader(GLuint &shader, GLenum shaderType, const GLchar *string)
{
  shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &string, NULL);
  glCompileShader(shader);
  GLint nResult = GL_FALSE;

  glGetShaderiv(shader, GL_COMPILE_STATUS, &nResult);

  if (!nResult)
  {
    IDBG_ERROR("Shader compilation status: %d", nResult);
    GLint nLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &nLength);
    if (nLength > 0)
    {
      char* pInfoLogStr = (char*)malloc(nLength);
      glGetShaderInfoLog(shader, nLength, NULL, pInfoLogStr);
      IDBG_ERROR("Shader compilation failure [%s]", pInfoLogStr);
      free(pInfoLogStr);
    }
    glDeleteShader(shader);
    return false;
  }
  return true;
}

bool CreateProgram(GLuint &program, GLuint vertex_shader,
  GLuint fragment_shader)
{
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);

  checkglError("Attach shaders");
  glLinkProgram(program);

  GLint nResult = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &nResult);
  checkglError("Link program");
  if (!nResult)
  {
    IDBG_ERROR("Shader link status %d", nResult);
    GLint nLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &nLength);
    if (nLength > 0)
    {
      char* pInfoLogStr = (char*)malloc(nLength);
      glGetProgramInfoLog(program, nLength, NULL, pInfoLogStr);
      IDBG_ERROR("Shader link status error [%s]", pInfoLogStr);
      free(pInfoLogStr);
    }
    glDeleteProgram(program);
    return false;
  }
  // Detach the shader after linking
  glDetachShader(program, vertex_shader);
  glDetachShader(program, fragment_shader);
  return true;
}

bool ReadProgram(const char *path, GLuint *program)
{
  FILE *fin = fopen(path, "rb");
  if (fin == NULL)
  {
      IDBG_LOW("Shader not loaded %s", path);
      return false;
  }

  char buffer[20000];
  GLenum binaryFormat = 34624;

  int c = 0;
  int len = 0;
  do
  {
      c = fgetc (fin);
      if (c != EOF)
      {
          buffer[len] = c;
          len++;
      }
  } while (c != EOF);

  fclose(fin);

  timeval t1, t2;
  gettimeofday(&t1, NULL);

  *program = glCreateProgram();
  glProgramBinary(*program, binaryFormat, buffer, len);

  GLint success = GL_FALSE;
  glGetProgramiv(*program, GL_LINK_STATUS, &success);

  gettimeofday(&t2, NULL);
  float elapsed_time = (t2.tv_sec - t1.tv_sec) * 1000.0f;  // sec -> ms
  elapsed_time += (t2.tv_usec - t1.tv_usec) / 1000.0f;  // us -> ms
  IDBG_LOW("Shader program load: %f", elapsed_time);

  return GL_TRUE == success;
}

void WriteProgram(GLuint program, const char *path)
{
  int program_length = 0;
  glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &program_length);
  IDBG_LOW("Program %d size %d", program, program_length);


  std::vector<char> buffer(program_length);
  GLenum format;  // = 34624
  glGetProgramBinary(program, program_length, NULL, &format, &buffer[0]);

  FILE *fout = fopen(path, "wb");

  if (!fout) {
    IDBG_ERROR("File open failed!");
    return;
  }

  for (int i = 0; i < program_length; ++i)
  {
      fputc(buffer[i], fout);
  }
  fclose(fout);
}

bool LoadProgram(GLuint &program)
{
  GLuint vertexShader = 0;
  GLuint fragmentShader = 0;

  bool load_binary = true;
  #if 0
  if (load_binary)
  {
    if (ReadProgram(QCAMERA_DUMP_FRM_LOCATION"camera_eis30_program.bin", &program))
    {
        IDBG_ERROR("Load shader success");
        return true;
    }
    else
    {
        IDBG_ERROR("No shader binary available. Compile it!");
        checkglError("Load GL programs");
    }
  }

  #endif
  timeval t1, t2;
  gettimeofday(&t1, NULL);

  GLint nResult;

  if (!CreateShader(vertexShader, GL_VERTEX_SHADER, vertexShaderSource))
  {
    IDBG_ERROR("CreateShader vertexShader failed");
    return false;
  }

  if (!CreateShader(fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderSource))
  {
    IDBG_ERROR("CreateShader fragmentShader failed");
    return false;
  }

  if (!CreateProgram(program, vertexShader, fragmentShader))
  {
    IDBG_ERROR("CreateProgram program failed");
    return false;
  }

  // Clean up. Delete linked shaders
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glFinish();

  gettimeofday(&t2, NULL);
  float elapsed_time = (t2.tv_sec - t1.tv_sec) * 1000.0f;  // sec -> ms
  elapsed_time += (t2.tv_usec - t1.tv_usec) / 1000.0f;  // us -> ms
  IDBG_LOW("Shader program init: %f", elapsed_time);

  //WriteProgram(program, "/data/vendor/camera/camera_eis30_program.bin");

  return true;
}

void enableTextureUnit(GLuint program, const char* image_unit_string,
  int unit_num)
{
  GLuint sampler = glGetUniformLocation(program, image_unit_string);
  glActiveTexture(unit_num);
  glUniform1i(sampler, unit_num - GL_TEXTURE0);
}


void ScaleAndCenter(float *matrix, int num_vertices, int width, int height,
  int centerX, int centerY, float width_ratio, float height_ratio)
{
  float w, h;
  float w1, h1, w2, h2;
  float w_adj, h_adj;


  w = (float) width;
  h = (float) height;
  w1 = 0.5f*width_ratio;
  h1 = 0.5f*height_ratio;
  w_adj = 0.00001f;
  h_adj = 0.00001f;
  float centerAdjustmentX = w1*(1.0f+ 1.0f/w) - w_adj;
  float centerAdjustmentY = h1*(1.0f + 1/h) - h_adj;

  float finalCenterAdjustmentX = (float)centerX/w;
  float finalCenterAdjustmentY = (float)centerY/h;

  int stride = num_vertices*3;
  for(int i=0; i<num_vertices; i++)
  {
    matrix[1] *= h/w;
    matrix[2] = matrix[2]/w;
    w2 = (float) (matrix[0]*centerAdjustmentX +
                  matrix[1]*centerAdjustmentY);
    matrix[2] -= w2;


    matrix[stride+0] *= w/h;
    matrix[stride+2] = matrix[stride+2]/h;
    h2 = (float) (matrix[stride+0]*centerAdjustmentX +
                  matrix[stride+1]*centerAdjustmentY);
    matrix[stride+2] -= h2;


    matrix[2*stride+0] *= w;
    matrix[2*stride+1] *= h;
    h2 = matrix[2*stride+0]*centerAdjustmentX+
      matrix[2*stride+1]*centerAdjustmentY;
    matrix[2*stride+2] -= h2;

    matrix[0] += finalCenterAdjustmentX*matrix[2*stride+0];
    matrix[1] += finalCenterAdjustmentX*matrix[2*stride+1];
    matrix[2] += finalCenterAdjustmentX*matrix[2*stride+2];
    matrix[stride+0] += finalCenterAdjustmentY*matrix[2*stride+0];
    matrix[stride+1] += finalCenterAdjustmentY*matrix[2*stride+1];
    matrix[stride+2] += finalCenterAdjustmentY*matrix[2*stride+2];

    /* print out matrix for testing purpose
    printf(
    "======================== start ============================\n"
    "row 1: %10.9f,  %10.9f  %10.9f\n"
    "row 2: %10.9f,  %10.9f  %10.9f\n"
    "row 3: %10.9f,  %10.9f  %10.9f\n"
    "======================== end ============================\n",
    matrix[0], matrix[1], matrix[2],
    matrix[stride+0], matrix[stride+1], matrix[stride+2],
    matrix[2*stride+0], matrix[2*stride+1], matrix[2*stride+2]); */

    matrix+=3;
  }
}

void getTransformationMatrices(float* matrixIn, float* matrixOut,
  int num_vertices, int texWidth, int texHeight, int centerX, int centerY,
  float width_ratio, float height_ratio)
{
  IDBG_LOW("num_vertices %d ", num_vertices);


  IDBG_LOW("Matrix 1 : %f %f %f | %f %f %f | %f %f %f ",
    matrixIn[0], matrixIn[1],matrixIn[2],
    matrixIn[3], matrixIn[4],matrixIn[5],
    matrixIn[6], matrixIn[7],matrixIn[8]);
  IDBG_LOW("Matrix 2 : %f %f %f | %f %f %f | %f %f %f ",
    matrixIn[9], matrixIn[10],matrixIn[11],
    matrixIn[12], matrixIn[13],matrixIn[14],
    matrixIn[15], matrixIn[16],matrixIn[17]);

  for (int i = 0,j = 0; i < num_vertices >> 1; i++, j+=9)
  {
    IDBG_LOW("i %d j %d", i, j);
    #if 1 //COL_MAJOR
  	matrixOut[i * 6 + 0] = matrixIn[j +0];
  	matrixOut[i * 6 + 1] = matrixIn[j +1];
  	matrixOut[i * 6 + 2] = matrixIn[j +2];

  	matrixOut[i * 6 + 3] = matrixIn[j +0];
  	matrixOut[i * 6 + 4] = matrixIn[j +1];
  	matrixOut[i * 6 + 5] = matrixIn[j +2];

  	matrixOut[num_vertices *3 + i * 6 + 0] = matrixIn[j +3];
  	matrixOut[num_vertices *3 + i * 6 + 1] = matrixIn[j +4];
  	matrixOut[num_vertices *3 + i * 6 + 2] = matrixIn[j +5];

  	matrixOut[num_vertices *3 + i * 6 + 3] = matrixIn[j +3];
  	matrixOut[num_vertices *3 + i * 6 + 4] = matrixIn[j +4];
  	matrixOut[num_vertices *3 + i * 6 + 5] = matrixIn[j +5];

  	matrixOut[num_vertices *6 + i * 6 + 0] = matrixIn[j +6];
  	matrixOut[num_vertices *6 + i * 6 + 1] = matrixIn[j +7];
  	matrixOut[num_vertices *6 + i * 6 + 2] = matrixIn[j +8];

  	matrixOut[num_vertices *6 + i * 6 + 3] = matrixIn[j +6];
  	matrixOut[num_vertices *6 + i * 6 + 4] = matrixIn[j +7];
  	matrixOut[num_vertices *6 + i * 6 + 5] = matrixIn[j +8];
    #endif
    #ifdef ROW_MAJOR
    matrixOut[i * 6 + 0] = matrixIn[j +0];
    matrixOut[i * 6 + 1] = matrixIn[j +3];
    matrixOut[i * 6 + 2] = matrixIn[j +6];

    matrixOut[i * 6 + 3] = matrixIn[j +0];
    matrixOut[i * 6 + 4] = matrixIn[j +3];
    matrixOut[i * 6 + 5] = matrixIn[j +6];

    matrixOut[num_vertices *3 + i * 6 + 0] = matrixIn[j +1];
    matrixOut[num_vertices *3 + i * 6 + 1] = matrixIn[j +4];
    matrixOut[num_vertices *3 + i * 6 + 2] = matrixIn[j +7];

    matrixOut[num_vertices *3 + i * 6 + 3] = matrixIn[j +1];
    matrixOut[num_vertices *3 + i * 6 + 4] = matrixIn[j +4];
    matrixOut[num_vertices *3 + i * 6 + 5] = matrixIn[j +7];

    matrixOut[num_vertices *6 + i * 6 + 0] = matrixIn[j +2];
    matrixOut[num_vertices *6 + i * 6 + 1] = matrixIn[j +5];
    matrixOut[num_vertices *6 + i * 6 + 2] = matrixIn[j +8];

    matrixOut[num_vertices *6 + i * 6 + 3] = matrixIn[j +2];
    matrixOut[num_vertices *6 + i * 6 + 4] = matrixIn[j +5];
    matrixOut[num_vertices *6 + i * 6 + 5] = matrixIn[j +8];
    #endif
  }

  for(int k = 0 ; k < 25 ; k+=12) {
    IDBG_LOW("Matrix col %d : %f %f %f | %f %f %f | %f %f %f | %f %f %f", k,
      matrixOut[k+0], matrixOut[k+1],matrixOut[k+2],
      matrixOut[k+3], matrixOut[k+4],matrixOut[k+5],
      matrixOut[k+6], matrixOut[k+7],matrixOut[k+8],
      matrixOut[k+9], matrixOut[k+10],matrixOut[k+11]);
  }

  ScaleAndCenter(matrixOut, num_vertices, texWidth, texHeight, centerX,
  centerY, width_ratio, height_ratio);
}

void generateVertexAndTextureCoordinates(float* vertices,
  float* texCoordinates, int num_vertices_x, int num_vertices_y,
  float width_ratio, float height_ratio)
{
  for(int y=0; y<num_vertices_y; y++)
  {
  for(int x=0; x<num_vertices_x; x++)
  {
  	vertices[4*y*num_vertices_x + 4*x + 0] =
            -1.0f + (2.0f*x)/(num_vertices_x-1);
  	vertices[4*y*num_vertices_x + 4*x + 1] =
            -1.0f + (2.0f*y)/(num_vertices_y-1);
  	vertices[4*y*num_vertices_x + 4*x + 2] = -0.5f;
  	vertices[4*y*num_vertices_x + 4*x + 3] = 1.0f;

  	texCoordinates[3*y*num_vertices_x + 3*x + 0] =
            (width_ratio*x)/(num_vertices_x-1);
  	texCoordinates[3*y*num_vertices_x + 3*x + 1] =
            (height_ratio*y)/(num_vertices_y-1);
  	texCoordinates[3*y*num_vertices_x + 3*x + 2] = 1.0f;
  }
  }
}

void generateIndices(unsigned short* indices, int num_mesh_x, int num_mesh_y)
{
  int num_vertices_x = (num_mesh_x+1);
  for(int y=0; y<num_mesh_y; y++)
  {
  	for(int x=0; x<num_mesh_x; x++)
  	{
  		indices[6*y*num_mesh_x + 6*x + 0] = y*num_vertices_x + x;
  		indices[6*y*num_mesh_x + 6*x + 1] = (y+1)*num_vertices_x + x;
  		indices[6*y*num_mesh_x + 6*x + 2] = y*num_vertices_x + (x+1);
  		indices[6*y*num_mesh_x + 6*x + 3] = (y+1)*num_vertices_x + x;
  		indices[6*y*num_mesh_x + 6*x + 4] = y*num_vertices_x + (x+1);
  		indices[6*y*num_mesh_x + 6*x + 5] = (y+1)*num_vertices_x + (x+1);
  	}
  }
}

void configureMatrixInfo(float* matrix, int num_vertices, GLint transId)
{
  IDBG_LOW("num_vertices %d ", num_vertices);
  glEnableVertexAttribArray(transId);
  glEnableVertexAttribArray(transId+1);
  glEnableVertexAttribArray(transId+2);
  glVertexAttribPointer(transId, 3, GL_FLOAT, GL_FALSE, 0, matrix);
  checkglError("glVertexAttribPointer");
  glVertexAttribPointer(transId+1, 3, GL_FLOAT, GL_FALSE, 0,
      matrix + num_vertices*3);
  glVertexAttribPointer(transId+2, 3, GL_FLOAT, GL_FALSE, 0,
      matrix + num_vertices*6);
}

void configureVertexAndTextureCoordinates(float* vertices,
  float* texCoordinates, GLint posId, GLint texId)
{
  glEnableVertexAttribArray(posId);
  glEnableVertexAttribArray(texId);
  glVertexAttribPointer(posId, 4, GL_FLOAT, GL_FALSE, 0, vertices);
  checkglError("glVertexAttribPointer vertices");
  glVertexAttribPointer(texId, 3, GL_FLOAT, GL_FALSE, 0, texCoordinates);
  checkglError("glVertexAttribPointer texCoords");

}

void allocIndices(GLuint *indexVBOID, int numTriangles,
  unsigned short *indices)
{
  glGenBuffers(1, indexVBOID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexVBOID);
  checkglError("glBindBuffer indices");
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, numTriangles*3*sizeof(unsigned short),
  indices, GL_STATIC_DRAW);
  checkglError("glBufferData indices");
}

void configureIndices(GLuint *indexVBOID)
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexVBOID);
  checkglError("glBindBuffer indices");
}

int ppeis_can_wait(void *p_userdata)
{
  QCameraPPEisCore *pCamPPEisCore = (QCameraPPEisCore*)p_userdata;
  if(!pCamPPEisCore) {
    return 0;
  }

  if (pCamPPEisCore->IsDeinitStarted()) {
    IDBG_MED(": ppeis_can_wait: CamPPEisCore Deinit started");
    return 0;
  }

  return 1;
}


void *ppeis_wait_thread_loop(void *handle)
{
  QCameraPPEisCore *pCamPPEisCore = (QCameraPPEisCore*)handle;
  img_msg_t *p_msg = NULL;
  pCamPPEisCore->m_pPpeisContext = NULL;

  while (TRUE) {
    IDBG_LOW(": E img_q_wait, callBackQ size %d",
    img_q_count(&pCamPPEisCore->callBackQ));
    pCamPPEisCore->m_pPpeisContext = (ppeis_context_t *)
      img_q_wait(&pCamPPEisCore->callBackQ, ppeis_can_wait, pCamPPEisCore);

    if (!ppeis_can_wait(pCamPPEisCore)) {
      break;
    }
    else if (!pCamPPEisCore->m_pPpeisContext) {
      continue;
    }

    IDBG_LOW("img_q_wait done");
    if(pCamPPEisCore->m_pPpeisContext->glSyncs != NULL) {
      IDBG_LOW("Waiting for eglMakeCurrent, glSyncs=%p, EglDisplay %p,\
        EglSurface %p, EglContext %p", pCamPPEisCore->m_pPpeisContext->glSyncs,
        pCamPPEisCore->m_pThreadData->display,
        pCamPPEisCore->m_pThreadData->surface,
        pCamPPEisCore->m_pThreadData->context);
      eglMakeCurrent(pCamPPEisCore->m_pThreadData->display,
        pCamPPEisCore->m_pThreadData->surface,
        pCamPPEisCore->m_pThreadData->surface,
        pCamPPEisCore->m_pThreadData->context);
      checkEglError("eglMakeCurrent");
      IDBG_LOW("eglMakeCurrent Done, Waiting for glClientWaitSync,\
        glSyncs=%p", pCamPPEisCore->m_pPpeisContext->glSyncs);
      glClientWaitSync(pCamPPEisCore->m_pPpeisContext->glSyncs, 0,
        GL_TIMEOUT_IGNORED);
      IDBG_MED("glClientWaitSync Done, and deleting glSyncs=%p",
        pCamPPEisCore->m_pPpeisContext->glSyncs);
      glDeleteSync(pCamPPEisCore->m_pPpeisContext->glSyncs);
      checkEglError("glDeleteSync");
      pCamPPEisCore->m_pPpeisContext->glSyncs = NULL;
      eglMakeCurrent(pCamPPEisCore->m_pThreadData->display, EGL_NO_SURFACE,
        EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    if (!pCamPPEisCore->callback ||
      !pCamPPEisCore->m_pPpeisContext->user_data) {
      IDBG_ERROR("invalid input arguments %p %p", pCamPPEisCore->callback,
        pCamPPEisCore->m_pPpeisContext->user_data);
      break;
    } else {
      IDBG_LOW("user_data %p %p", pCamPPEisCore->callback,
        pCamPPEisCore->m_pPpeisContext->user_data);
      pCamPPEisCore->callback(pCamPPEisCore->m_pPpeisContext->user_data);
      pCamPPEisCore->m_pPpeisContext->user_data = NULL;
      free(pCamPPEisCore->m_pPpeisContext);
      pCamPPEisCore->m_pPpeisContext = NULL;
    }
    IDBG_LOW(": X");
  }
  IDBG_LOW("Exiting threadId :%d", pCamPPEisCore->threadId);
  return NULL;
}

/**
 * Function: img_algo_init
 *
 * Description: algorirhm initialization
 *
 * Arguments:
 *   @pp_context: return context [Output parameter]
 *   @p_params: init params
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_init(void **p_context, img_init_params_t *p_params)
{
  IDBG_LOW(": E");
  int ret = IMG_ERR_GENERAL;
  IMG_UNUSED(p_params);

  QCameraPPEisCore *pCamPPEisCore;
  pCamPPEisCore = new QCameraPPEisCore;
  if (pCamPPEisCore) {
    *p_context = pCamPPEisCore;
    if (pCamPPEisCore->Init()) {
      img_q_init(&pCamPPEisCore->callBackQ, (char *)"PPEISFrameCbQ");
      ret = IMG_SUCCESS;
    } else {
      (void)pCamPPEisCore->DeInit();
    }
  }
  IDBG_LOW(": X");
  return ret;
}

/**
 * Function: img_algo_deinit
 *
 * Description: algorithm de-initialization
 *
 * Arguments:
 *   @arg1:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_deinit(void *p_context)
{
  IDBG_LOW("img_algo_deinit called");
  assert(NULL != p_context);

  QCameraPPEisCore* pCamPPEisCore = (QCameraPPEisCore*) p_context;
  if (pCamPPEisCore) {
    (void)pCamPPEisCore->DeInit();
    img_q_deinit(&pCamPPEisCore->callBackQ);
    delete pCamPPEisCore;
    pCamPPEisCore = NULL;
  }

  IDBG_LOW(": X");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_deinit
 *
 * Description: algorithm de-initialization
 *
 * Arguments:
 *   @arg1:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_stop(void *p_context)
{
  IDBG_LOW("img_algo_stop called");
  assert(NULL != p_context);

  QCameraPPEisCore* pCamPPEisCore = (QCameraPPEisCore*) p_context;
  if (pCamPPEisCore) {
    pCamPPEisCore->Flush();
  }
  IDBG_LOW(": X");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_set_callback
 *
 * Description: Set callback pointer
 *
 * Arguments:
 *   @p_context: class pointer
 *   @algo_callback: user callback function
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
extern "C" int img_algo_set_callback(void *p_context,
  int (*algo_callback)(void *user_data))
{
  assert(NULL != p_context);
  QCameraPPEisCore *pCamPPEisCore = (QCameraPPEisCore *)p_context;
  int ret = IMG_SUCCESS;

  if (!pCamPPEisCore || !algo_callback) {
    IDBG_ERROR(": failed: %p %p", pCamPPEisCore, algo_callback);
    return IMG_ERR_INVALID_INPUT;
  }
  pCamPPEisCore->setDeinitStart(false);
  pCamPPEisCore->callback = algo_callback;
  int retval = pthread_create(&pCamPPEisCore->threadId, NULL,
    ppeis_wait_thread_loop, (void*)p_context);
  IDBG_LOW("PPEIS_wait_thread created threadId: %d",pCamPPEisCore->threadId);
  pthread_setname_np(pCamPPEisCore->threadId, "PPEIS_wait_thread");
  if (retval < 0) {
    IDBG_ERROR("PPEIS_wait_thread creation failed");
    return IMG_ERR_GENERAL;
  } else {
    IDBG_MED("PPEIS_wait_thread creation success");
  }

  return IMG_SUCCESS;
}



/**
 * Function: img_algo_process
 *
 * Description: algorithm main body
 *
 * Arguments:
 *   @arg1: p_context: class pointer
 *   @arg2: p_in_frame: input frame handler
 *   @arg3: in_frame_cnt: number of input frames
 *   @arg4: p_out_frame: output frame handler
 *   @arg5: out_frame_cnt: number of output frames
 *   @arg6: p_meta: frame meta data
 *   @arg7: meta_cnt: number of meta data array
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
extern "C" int img_algo_process(void *p_context, img_frame_t *p_in_frame[],
  int in_frame_cnt,img_frame_t *p_out_frame[],
  int out_frame_cnt, img_meta_t *p_meta[], int meta_cnt)
{
  IDBG_LOW("PPEisCore ProcessFrame %d, %d", in_frame_cnt, out_frame_cnt);

  int ret = IMG_SUCCESS;
  if (p_context != NULL && p_meta != NULL) {
    QCameraPPEisCore* pCamPPEisCore = (QCameraPPEisCore*) p_context;
    ret = pCamPPEisCore->processData(p_in_frame, in_frame_cnt, p_out_frame,
      out_frame_cnt, (p_meta && (meta_cnt > 0)) ? p_meta[0] : NULL);
  } else {
    IDBG_ERROR(" img_algo_process failed");
    ret = IMG_ERR_GENERAL;
  }

  IDBG_LOW(": X");

  return ret;
}

/**
 * Function: img_algo_set_frame_ops
 *
 * Description: Function to set frame operations
 *
 * Arguments:
 *   @p_context: ptr to instance
 *   @p_ops: ptr to frame ops
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_set_frame_ops(void *p_context,
  img_frame_ops_t *p_ops)
{
  IDBG_LOW(": E");
  int ret = IMG_SUCCESS;
  assert(NULL != p_context);
  assert(NULL != p_ops);
  QCameraPPEisCore* instance = (QCameraPPEisCore*) p_context;
  instance->setFrameOps(p_ops);
  IDBG_LOW(": X");
  return ret;
}

/**
 * Function: QCameraPPEisCore
 *
 * Description: constructor of class
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
QCameraPPEisCore::QCameraPPEisCore()
{
  IDBG_LOW(": E");

  memset(&mFrameOps, 0x0, sizeof(mFrameOps));
  mBufCnt = 0;
  mPPEisCoreInitialized = false;
  mShaderProgram = 0;
  mPosId = 0;
  mTexId = 0;
  mTransId = 0;
  mVertices = NULL;
  mTexCoordinates = NULL;
  mIndices = NULL;
  mIndexVBOID = 0;
  m_pThreadData = NULL;
  cfg = {DEFAULT_WIDTH, DEFAULT_HEIGHT, 8, 8, 8, 8, 32, 24, 8, 0, 0};
  memset(&m_pThreadData, 0x0, sizeof(m_pThreadData));
  IDBG_LOW(": X");
}

/**
 * Function: ~QCameraPPEisCore
 *
 * Description: destructor of class
 *
 * Arguments:
 *   @arg1:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
QCameraPPEisCore::~QCameraPPEisCore()
{

}

/**
 * Function: QCameraPPEisCore: Init
 *
 * Description: Init of class
 *
 * Arguments:
 *   @arg1:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
bool QCameraPPEisCore::Init()
{
  bool ret = false;
  GLint posId, texId, transId;

  IDBG_LOW("E");

  mBufferMap.Init();
  m_defaultDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(m_defaultDisplay, &majorVersion, &minorVersion);

  //----------------------------
  // Create default context data
  //----------------------------
  ret =  CreateContext(m_defaultDisplay, NULL, &m_defaultSurface,
    &m_defaultContext);
  if (ret == true) {
    m_pThreadData = new (ppeis_thread_data_t);

    if (m_pThreadData != NULL) {
      m_pThreadData->display = m_defaultDisplay;

      //---------------------------
      // Create thread context data
      //---------------------------
      ret =  CreateContext(m_pThreadData->display, m_defaultContext,
        &m_pThreadData->surface, &m_pThreadData->context);

      if (ret == true) {
        if(!eglMakeCurrent(m_defaultDisplay, m_defaultSurface, m_defaultSurface,
          m_defaultContext)) {
          IDBG_ERROR("eglMakeCurrent failed with error code: %x",eglGetError());
          ret = false;
        } else {
          IDBG_LOW("LoadProgram");
          ret = LoadProgram(mShaderProgram);
          if (ret == true) {
            glUseProgram(mShaderProgram);
            mPosId = glGetAttribLocation(mShaderProgram, "inPosition");
            mTexId = glGetAttribLocation(mShaderProgram, "inTexCoord");
            mTransId = glGetAttribLocation(mShaderProgram, "inTransform");
            IDBG_LOW("posId %d texId %d transId %d", mPosId, mTexId, mTransId);
            glBindAttribLocation(mShaderProgram, mPosId, "inPosition");
            checkglError("glBindAttribLocation");
            glBindAttribLocation(mShaderProgram, mTexId, "inTexCoord");
            checkglError("glBindAttribLocation");
            glBindAttribLocation(mShaderProgram, mTransId, "inTransform");
            checkglError("glBindAttribLocation");
            enableTextureUnit(mShaderProgram, "tex", GL_TEXTURE0);
            // Reset to null
            eglMakeCurrent(m_defaultDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
              EGL_NO_CONTEXT);
            checkEglError("eglMakeCurrent");
          }
        }
      }
    }
  }
  return ret;
}


/**
 * Function: QCameraPPEisCore::CreateContext
 *
 * Description: Created context
 *
 * Arguments:
 *
 * Return values:
 *     bool
 *
 * Notes: none
 **/

bool QCameraPPEisCore::CreateContext(EGLDisplay display,
  EGLContext   sharedContext,
  EGLSurface   *surface,
  EGLContext   *context)
{
  int ret = false;
  static EGLint PbufferConfigAttribs[] =
  {
    EGL_RED_SIZE,       8,
    EGL_GREEN_SIZE,     8,
    EGL_BLUE_SIZE,      8,
    EGL_ALPHA_SIZE,     8,
    EGL_DEPTH_SIZE,     24,
    EGL_STENCIL_SIZE,   8,
    EGL_SURFACE_TYPE,   EGL_PBUFFER_BIT,
    EGL_SAMPLE_BUFFERS, 0,
    EGL_SAMPLES,        0,
    EGL_NONE
  };

  static const EGLint ContextAttribs[] =
  {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };

  static EGLint s_pbufferAttribs[] =
  {
    EGL_WIDTH,  0,
    EGL_HEIGHT, 0,
    EGL_NONE
  };

  s_pbufferAttribs[1] = cfg.winW;
  s_pbufferAttribs[3] = cfg.winH;
  eglBindAPI(EGL_OPENGL_ES_API);
  IDBG_LOW("EGL Vendor: %s ", eglQueryString(m_defaultDisplay,EGL_VENDOR));
  IDBG_LOW("EGL Version: %d:%d ", majorVersion, minorVersion);

  config = GetEglConfig(m_defaultDisplay, PbufferConfigAttribs, &cfg);
  *context = eglCreateContext(display, config, sharedContext,
    ContextAttribs);
  if(*context == EGL_NO_CONTEXT) {
    IDBG_ERROR("Unable to create EGL context");
    return ret;
  }
  *surface= eglCreatePbufferSurface(m_defaultDisplay, config,
    s_pbufferAttribs);
  if(*surface == EGL_NO_SURFACE) {
    IDBG_ERROR("Unable to create EGL surface");
    return ret;
  }
  IDBG_MED("EglDisplay %p, EglSurface %p, EglContext %p",
    display, *surface, *context);
  ret = true;
  return ret;
}


/**
 * Function: QCameraPPEisCore::Flush
 *
 * Description: DeInit function of class
 *
 * Arguments:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void QCameraPPEisCore::Flush()
{
  eglMakeCurrent(m_defaultDisplay, m_defaultSurface, m_defaultSurface,
    m_defaultContext);
  checkEglError("eglMakeCurrent");

  mBufferMap.FlushMaps();
  eglMakeCurrent(m_defaultDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
    EGL_NO_CONTEXT);
  checkEglError("eglMakeCurrent");
  IDBG_LOW("Last frame: Done Flushing Map");

   setDeinitStart(true);
   img_q_signal(&callBackQ);
   IDBG_LOW("before pthread_join threadId %d", threadId);
   pthread_join(threadId, NULL);
   IDBG_LOW("after pthread_join  threadId %d  Done", threadId);
}


/**
 * Function: QCameraPPEisCore::deInit
 *
 * Description: DeInit function of class
 *
 * Arguments:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
bool QCameraPPEisCore::DeInit()
{
  bool ret = TRUE;
  eglMakeCurrent(m_defaultDisplay, m_defaultSurface, m_defaultSurface,
    m_defaultContext);
  IDBG_LOW("E");
  glDisableVertexAttribArray(mPosId);
  glDisableVertexAttribArray(mTexId);
  glDisableVertexAttribArray(mTransId);
  glDisableVertexAttribArray(mTransId+1);
  glDisableVertexAttribArray(mTransId+2);

  //Reset program
  glUseProgram(0);

  //Delete shader program
  glDeleteProgram(mShaderProgram);

  //Deint buffer maps
  mBufferMap.DeInit();
  mPosId = 0;
  mTexId = 0;
  mTransId= 0;
  glDeleteBuffers(1, &mIndexVBOID);
  mIndexVBOID = 0;
  if (mVertices) {
    free(mVertices);
    mVertices = NULL;
  }
  if (mTexCoordinates) {
    free(mTexCoordinates);
    mTexCoordinates = NULL;
  }
  if (mIndices) {
    free(mIndices);
    mIndices = NULL;
  }

  // Reset Thread context to null
  if (m_pThreadData != NULL) {

    if (m_pThreadData->context != EGL_NO_CONTEXT) {
      eglDestroyContext(m_pThreadData->display,m_pThreadData->context);
      m_pThreadData->context = EGL_NO_CONTEXT;
    }
    if (m_pThreadData->surface != EGL_NO_SURFACE) {
      eglDestroySurface(m_pThreadData->display, m_pThreadData->surface);
      m_pThreadData->surface = EGL_NO_SURFACE;
    }
    if (m_pThreadData->display != EGL_NO_DISPLAY) {
      m_pThreadData->display = EGL_NO_DISPLAY;
    }
    delete m_pThreadData;
    m_pThreadData = NULL;
  }


  // Reset default context to null
  if (m_defaultContext != EGL_NO_CONTEXT) {
    eglDestroyContext(m_defaultDisplay,m_defaultContext);
    m_defaultContext = EGL_NO_CONTEXT;
  }
  if (m_defaultSurface != EGL_NO_SURFACE) {
    eglDestroySurface(m_defaultDisplay, m_defaultSurface);
    m_defaultSurface = EGL_NO_SURFACE;
  }
  if (m_defaultDisplay != EGL_NO_DISPLAY) {
    eglMakeCurrent(m_defaultDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
     EGL_NO_CONTEXT);
    eglTerminate(m_defaultDisplay);
    m_defaultDisplay = EGL_NO_DISPLAY;
  }

  callback = NULL;

  return ret;
}

/**
 * Function: processData
 *
 * Description: main processing body
 *
 * Arguments:
 *   @arg1: p_in_frame: input frame handler
 *   @arg2: in_frame_cnt: number of input frames
 *   @arg3: p_out_frame: output frame handler
 *   @arg4: out_frame_cnt: number of output frames
 *   @arg3: p_meta: frame meta data, containing ROI info
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int8_t QCameraPPEisCore::processData(img_frame_t *inFrame[],
  int inFrameCnt, img_frame_t *outFrame[], int outFrameCnt,
  img_meta_t *p_meta)
{
  int status = TRUE;
  int32_t update_format;
  int32_t rc = IMG_SUCCESS;
  ppeis_context_t *ppeis_context = NULL;

  if (!p_meta || !p_meta->userdata) {
    IDBG_ERROR("meta is NULL or userdata is NULL!");
    return IMG_ERR_GENERAL;
  }

  IDBG_LOW(": E");
  IDBG_LOW("frame_id %d lastframe = %d, %d x %d", p_meta->frame_id,
  p_meta->eis_cfg.last_frame, p_meta->eis_cfg.num_mesh_x,
    p_meta->eis_cfg.num_mesh_y);
  IDBG_LOW("inFrameCnt %d outFrameCnt %d ", inFrameCnt, outFrameCnt);

  int num_mesh_x = p_meta->eis_cfg.num_mesh_x;
  int num_mesh_y = p_meta->eis_cfg.num_mesh_y;
  int num_vertices_x = (num_mesh_x + 1);
  int num_vertices_y = (num_mesh_y + 1);
  int num_vertices = num_vertices_x * num_vertices_y;
  int numTriangles, texWidth, texHeight, centerX, centerY;

  float* transformMatrices = (float*)malloc(num_vertices*sizeof(float)*9);
  if(!transformMatrices) {
    IDBG_ERROR("transformMatrices is null!");
    return IMG_ERR_GENERAL;
  }

  GLuint sourceTexture = 0;
  GLuint frameBuffer = 0;
  float width_ratio = (float)QIMG_WIDTH(outFrame[0], 0)/
                         (float)QIMG_WIDTH(inFrame[0], 0);
  float height_ratio = (float)QIMG_HEIGHT(outFrame[0], 0)/
                         (float)QIMG_HEIGHT(inFrame[0], 0);

  numTriangles = num_mesh_x * num_mesh_y * 2;
  texWidth = QIMG_WIDTH(inFrame[0], 0);
  texHeight = QIMG_HEIGHT(inFrame[0], 0);
  centerX = QIMG_WIDTH(inFrame[0], 0)/2;
  centerY = QIMG_HEIGHT(inFrame[0], 0)/2;

  IDBG_MED("EglDisplay %p, EglSurface %p, EglContext %p",
  m_pThreadData->display, m_pThreadData->surface, m_pThreadData->context);


  eglMakeCurrent(m_defaultDisplay, m_defaultSurface, m_defaultSurface,
  m_defaultContext);
  checkEglError("eglMakeCurrent");

  glUseProgram(mShaderProgram);
  checkglError("glUseProgram");

  enableTextureUnit(mShaderProgram, "tex", GL_TEXTURE0);
  glViewport(0,0, QIMG_WIDTH(outFrame[0], 0), QIMG_HEIGHT(outFrame[0], 0));

  #if 0  //Debug logging
  IDBG_LOW(": E");
  IDBG_ERROR(" frameid %d I[%p, %d %d, %d %d, %d, type %d] "
      "O[%p, %d %d, %d %d, %d, type %d]", p_meta->frame_id,
      QIMG_ADDR(inFrame[0], 0), QIMG_WIDTH(inFrame[0], 0),
      QIMG_HEIGHT(inFrame[0], 0), QIMG_STRIDE(inFrame[0], 0),
      QIMG_SCANLINE(inFrame[0], 0), QIMG_LEN(inFrame[0], 0),
      QIMG_PL_TYPE(inFrame[0], 0), QIMG_ADDR(outFrame[0], 0),
      QIMG_WIDTH(outFrame[0], 0), QIMG_HEIGHT(outFrame[0], 0),
      QIMG_STRIDE(outFrame[0], 0), QIMG_SCANLINE(outFrame[0], 0),
      QIMG_LEN(outFrame[0], 0), QIMG_PL_TYPE(outFrame[0], 0));

  IDBG_ERROR(" I[%p, %d %d, %d %d, %d, type %d] "
      "O[%p, %d %d, %d %d, %d, type %d]",
      QIMG_ADDR(inFrame[0], 1), QIMG_WIDTH(inFrame[0], 1),
      QIMG_HEIGHT(inFrame[0], 1), QIMG_STRIDE(inFrame[0], 1),
      QIMG_SCANLINE(inFrame[0], 1), QIMG_LEN(inFrame[0], 1),
      QIMG_PL_TYPE(inFrame[0], 1), QIMG_ADDR(outFrame[0], 1),
      QIMG_WIDTH(outFrame[0], 1), QIMG_HEIGHT(outFrame[0], 1),
      QIMG_STRIDE(outFrame[0], 1), QIMG_SCANLINE(outFrame[0], 1),
      QIMG_LEN(outFrame[0], 1), QIMG_PL_TYPE(outFrame[0], 1));


  IDBG_LOW("Matrix 1 : %f %f %f | %f %f %f | %f %f %f ",
    p_meta->eis_cfg.transform_matrix[0], p_meta->eis_cfg.transform_matrix[1],
    p_meta->eis_cfg.transform_matrix[2], p_meta->eis_cfg.transform_matrix[3],
    p_meta->eis_cfg.transform_matrix[4], p_meta->eis_cfg.transform_matrix[5],
    p_meta->eis_cfg.transform_matrix[6], p_meta->eis_cfg.transform_matrix[7],
    p_meta->eis_cfg.transform_matrix[8]);
  IDBG_LOW("Matrix 2 : %f %f %f | %f %f %f | %f %f %f ",
    p_meta->eis_cfg.transform_matrix[9], p_meta->eis_cfg.transform_matrix[10],
    p_meta->eis_cfg.transform_matrix[11], p_meta->eis_cfg.transform_matrix[12],
    p_meta->eis_cfg.transform_matrix[13], p_meta->eis_cfg.transform_matrix[14],
    p_meta->eis_cfg.transform_matrix[15], p_meta->eis_cfg.transform_matrix[16],
    p_meta->eis_cfg.transform_matrix[17]);
  #endif
  //Setup vertices and texCoordinates. Once only.
  if (!mVertices || !mTexCoordinates) {
    mVertices = (float*)malloc(num_vertices*sizeof(float)*4);
    mTexCoordinates = (float*)malloc(num_vertices*sizeof(float)*3);
    if(!mVertices || !mTexCoordinates) {
      free(transformMatrices);
      IDBG_ERROR("mVertices or mTexCoordinates is null!");
      return IMG_ERR_GENERAL;
    }
    generateVertexAndTextureCoordinates(mVertices, mTexCoordinates,
      num_vertices_x, num_vertices_y, width_ratio, height_ratio);

    IDBG_LOW(" verts %f %f %f %f | %f %f %f %f | %f %f %f %f| %f %f %f %f",
      mVertices[0], mVertices[1], mVertices[2], mVertices[3], mVertices[4],
      mVertices[5], mVertices[6], mVertices[7], mVertices[8], mVertices[9],
      mVertices[10], mVertices[11], mVertices[12], mVertices[13],
      mVertices[14], mVertices[15]);
    IDBG_LOW(" texCo %f %f %f |%f %f %f| %f %f %f |%f %f %f ",
      mTexCoordinates[0], mTexCoordinates[1], mTexCoordinates[2],
      mTexCoordinates[3], mTexCoordinates[4], mTexCoordinates[5],
      mTexCoordinates[6], mTexCoordinates[7], mTexCoordinates[8],
      mTexCoordinates[9],mTexCoordinates[10],mTexCoordinates[11]);
  }

  //Setup indices. Once only
  if (!mIndexVBOID) {
    mIndices = (unsigned short*)malloc(numTriangles*3*sizeof(unsigned short));
    if(!mIndices) {
      free(transformMatrices);
      IDBG_ERROR("mIndices is null!");
      return IMG_ERR_GENERAL;
    }

    generateIndices(mIndices, num_mesh_x, num_mesh_y);
    IDBG_LOW("index %d %d %d %d %d %d", mIndices[0], mIndices[1], mIndices[2],
        mIndices[3],mIndices[4],mIndices[5]);
    allocIndices(&mIndexVBOID, numTriangles, mIndices);
    #if 0 //Enable but done below
    configureIndices(&mIndexVBOID, numTriangles, mIndices);
    #endif
    //free(indices);
  }
  //Setup Matrix information
  getTransformationMatrices(p_meta->eis_cfg.transform_matrix,
  transformMatrices, num_vertices, texWidth, texHeight, centerX, centerY,
  width_ratio, height_ratio);

  //Configure vertices, indices, matrices
  configureMatrixInfo(transformMatrices, num_vertices, mTransId );
  configureVertexAndTextureCoordinates(mVertices, mTexCoordinates, mPosId,
    mTexId);
  configureIndices(&mIndexVBOID);
  rc = img_common_get_frame_fmt(outFrame[0]->info.fmt, &update_format);
  if (IMG_ERROR(rc)) {
    free(transformMatrices);
    IDBG_ERROR("Invalid format = %d",outFrame[0]->info.fmt);
    return IMG_ERR_INVALID_INPUT;
  }

  //Setup buffer information
  sourceTexture = mBufferMap.GetSourceTexture(inFrame[0]->idx ,
    IMG_FD(inFrame[0]), QIMG_LEN(inFrame[0], 0) + QIMG_LEN(inFrame[0], 1),
    update_format,
    QIMG_WIDTH(inFrame[0], 0), QIMG_HEIGHT(inFrame[0], 0),
    QIMG_STRIDE(inFrame[0], 0), QIMG_SCANLINE(inFrame[0], 0));
  frameBuffer = mBufferMap.GetFrameBuffer(outFrame[0]->idx ,
    IMG_FD(outFrame[0]), QIMG_LEN(outFrame[0], 0) + QIMG_LEN(outFrame[0], 1),
    update_format,
    QIMG_WIDTH(outFrame[0], 0), QIMG_HEIGHT(outFrame[0], 0),
    QIMG_STRIDE(outFrame[0], 0), QIMG_SCANLINE(outFrame[0], 0));
  IDBG_LOW("sourceTexture %d frameBuffer %d", sourceTexture, frameBuffer);

  //Bind Source Texture and Frame Buffer
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, sourceTexture);
  checkglError("glBindTexture");

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
  checkglError("glBindFramebuffer");

  glScissor(0, 0, QIMG_WIDTH(outFrame[0], 0), QIMG_HEIGHT(outFrame[0], 0));
  checkglError("glDrawElements");

  glDrawElements( GL_TRIANGLES, 3*numTriangles, GL_UNSIGNED_SHORT, NULL);
  checkglError("glScissor");

  glFlush();

  if (!callback) {
    IDBG_ERROR("callback is NULL!");
    glFinish();
  } else {
    ppeis_context = (ppeis_context_t*)malloc(sizeof(ppeis_context_t));
    if (!ppeis_context) {
      IDBG_ERROR(" X  ppeis_context NULL");
      free(transformMatrices);
      return IMG_ERR_NO_MEMORY;
    }

    ppeis_context->user_data = p_meta->userdata;
    ppeis_context->glSyncs = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    checkglError("glSyncs");
    status = img_q_enqueue(&callBackQ, ppeis_context);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Cannot enqueue fence sync");
    } else {
      IDBG_MED("Successfully enqueue fence sync, glSyncs=%p",
        ppeis_context->glSyncs);
      img_q_signal(&callBackQ);
    }
  }

  glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
  checkglError("glBindTexture");

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  checkglError("glBindFramebuffer0");


  eglMakeCurrent(m_pThreadData->display, EGL_NO_SURFACE, EGL_NO_SURFACE,
    EGL_NO_CONTEXT);
  checkEglError("eglMakeCurrent");

  #if 0
  //dump_matrix(p_meta);
  //eis_dump_frame(inFrame[0], p_meta);5
  //int ret = mFrameOps.image_copy(outFrame[0], inFrame[0]);
  //int ret = image_crop(outFrame[0], inFrame[0]);
	//mFrameOps.dump_frame(inFrame[0], "ppeis_input", 0, NULL);
  //mFrameOps.dump_frame(outFrame[0], "ppeis_output", 0, NULL);
  #endif

  //Update timestamp
  outFrame[0]->timestamp = inFrame[0]->timestamp;
  free(transformMatrices);
  IDBG_LOW(": X ret");
  return IMG_SUCCESS;
}
