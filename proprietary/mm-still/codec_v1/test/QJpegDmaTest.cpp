/*****************************************************************************
* Copyright (c) 2012-2015,2017 Qualcomm Technologies, Inc.  All Rights Reserved.  *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QJpegDmaTest.h"
#include "QITime.h"

#define MAX_DOWNSCALE (16.0)

#define MAX_COLOR_FMTS 17

#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define CLAMP(x, min, max) MIN(MAX((x), (min)), (max))

/** dma_color_format_t:
 *
 * test color format mapping
 **/
typedef struct {
  char *format_str;
  QIFormat eColorFormat;
  float chroma_wt;
  int planeCnt;
  QISubsampling ss;
} dma_color_format_t;

/** col_formats:
 *
 * Color format mapping from testapp to OMX
 **/
static const dma_color_format_t col_formats[MAX_COLOR_FMTS] =
{
  { (char*)"YCRCBLP_H2V2",  QI_YCRCB_SP, 1.5, 2, QI_H2V2 },
  { (char*)"YCBCRLP_H2V2",  QI_YCBCR_SP, 1.5, 2, QI_H2V2 },
  { (char*)"YCRCBLP_H2V1",  QI_YCRCB_SP, 2.0, 2, QI_H2V1 },
  { (char*)"YCBCRLP_H2V1",  QI_YCBCR_SP, 2.0, 2, QI_H2V1 },
  { (char*)"YCRCBLP_H1V2",  QI_YCRCB_SP, 2.0, 2, QI_H1V2 },
  { (char*)"YCBCRLP_H1V2",  QI_YCBCR_SP, 2.0, 2, QI_H1V2 },
  { (char*)"YCRCBLP_H1V1",  QI_YCRCB_SP, 3.0, 2, QI_H1V1 },
  { (char*)"YCBCRLP_H1V1",  QI_YCBCR_SP, 3.0, 2, QI_H1V1 },
  { (char*)   "IYUV_H2V2",  QI_IYUV, 1.5, 3, QI_H2V2 },
  { (char*)   "YUV2_H2V2",  QI_YUV2, 1.5, 3, QI_H2V2 },
  { (char*)   "IYUV_H2V1",  QI_IYUV, 2.0, 3, QI_H2V1 },
  { (char*)   "YUV2_H2V1",  QI_YUV2, 2.0, 3, QI_H2V1 },
  { (char*)   "IYUV_H1V2",  QI_IYUV, 2.0, 3, QI_H1V2 },
  { (char*)   "YUV2_H1V2",  QI_YUV2, 2.0, 3, QI_H1V2 },
  { (char*)   "IYUV_H1V1",  QI_IYUV, 3.0, 3, QI_H1V1 },
  { (char*)   "YUV2_H1V1",  QI_YUV2, 3.0, 3, QI_H1V1 },
  { (char*)  "MONOCHROME",  QI_MONOCHROME, 1.0, 1, QI_H1V1 }
};



/*===========================================================================
 * Function: QJpegDmaTest
 *
 * Description: QJpegDmaTest default constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJpegDmaTest::QJpegDmaTest()
{
  mpDma = NULL;
  mOutputFilename = NULL;
  mInputFilename = NULL;
  mInput = NULL;
  mOutput = NULL;
  mInputdata = NULL;
  mOutputdata = NULL;
  mSS = QI_H2V2;
  mImageSize = 0;
  mOutputSize = 0;
  QI_MUTEX_INIT(&mMutex);
  QI_COND_INIT(&mCond);
  mError = QI_SUCCESS;
  mFormat = QI_YCRCB_SP;
  mTimeout = 30000;
  mTimeoutFlag = 0;
  mCropFlag = false;
}


/*===========================================================================
 * Function: ~QJpegDmaTest
 *
 * Description: QJpegDmaTest destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJpegDmaTest::~QJpegDmaTest()
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  if (mpDma) {
    delete mpDma;
    mpDma = NULL;
  }
  if (mInput) {
    delete mInput;
    mInput = NULL;
  }
  if (mOutput) {
    delete mOutput;
    mOutput = NULL;
  }
  if (mInputdata) {
    delete mInputdata;
    mInputdata = NULL;
  }
  if (mOutputdata) {
    delete mOutputdata;
    mOutputdata = NULL;
  }

  QI_MUTEX_DESTROY(&mMutex);
  QI_COND_DESTROY(&mCond);
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*===========================================================================
 * Function: Init
 *
 * Description: initializate the dma test object
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QJpegDmaTest::Init()
{
  QIDBG_MED("%s:%d] %p", __func__, __LINE__, this);

  mImageSize = QImage::getImageSize(mSize, mSS, mFormat);
  QIDBG_HIGH("%s:%d] mImageSize %d %d", __func__, __LINE__, mImageSize,
    mSize.Length());

  mImageOutSize = QImage::getImageSize(mOutSize, mSS, mFormat);
  QIDBG_HIGH("%s:%d] mImageSize %d %d", __func__, __LINE__, mImageSize,
    mSize.Length());
  mInputdata = QIONBuffer::New(mImageSize, false);
  if (mInputdata == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  mOutputdata = QIONBuffer::New(mImageOutSize, false);
  if (mOutputdata == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Read
 *
 * Description: read the data from the file and fill the buffers
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QJpegDmaTest::Read()
{
  FILE *fp = fopen(mInputFilename, "rb");
  int lrc = 0;
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  if (fp) {
    lrc = fread(mInputdata->Addr(), 1, mImageSize, fp);
    QIDBG_HIGH("%s:%d] bytes_read %d", __func__, __LINE__, lrc);
    mInputdata->SetFilledLen(mImageSize);
    fclose(fp);
  } else {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Write
 *
 * Description: write the filled data to the file
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QJpegDmaTest::Write()
{
  FILE *fp = fopen(mOutputFilename, "w+");
  int lrc = 0;
  mOutputSize = mOutput->FilledLen() + mOutputdata->FilledLen();
  QIDBG_MED("%s:%d] filled len %d", __func__, __LINE__, mOutputSize);
  if (fp) {
    lrc = fwrite(mOutputdata->Addr(), 1, mOutputSize, fp);
    QIDBG_HIGH("%s:%d] bytes_written %d", __func__, __LINE__, lrc);
    fclose(fp);
  } else {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}


/*===========================================================================
 * Function: Start
 *
 * Description: start jpeg dma
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_NO_MEMORY
 *
 * Notes: none
 *==========================================================================*/
int QJpegDmaTest::Start()
{
  int lrc = QI_SUCCESS;
  uint32_t lInOffset[QI_MAX_PLANES], lInPhyOffset[QI_MAX_PLANES] = {0,
      0, 0};
  uint32_t lOutOffset[QI_MAX_PLANES], lOutPhyOffset[QI_MAX_PLANES] = {0,
      0, 0};

  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  /* create dma*/
  mpDma = QJPEGDMAEngine::New(NULL, NULL);

  if (mpDma == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  mInput = new QImage(mSize, mSS, mFormat);
  if (mInput == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  lInPhyOffset[0] = 0;
  lInPhyOffset[1] = mSize.Length();
  lInPhyOffset[2] = lInPhyOffset[1];

  lrc = mInput->setDefaultPlanes(2, mInputdata->Addr(), mInputdata->Fd(),
      lInOffset, lInPhyOffset);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  mOutput = new QImage(mOutSize, mSS, mFormat);
  if (mOutput == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  mOutputdata->SetFilledLen(mImageOutSize);

  lOutPhyOffset[0] = 0;
  lOutPhyOffset[1] = mOutSize.Length();
  lOutPhyOffset[2] = lOutPhyOffset[1];
  lrc = mOutput->setDefaultPlanes(2, mOutputdata->Addr(), mOutputdata->Fd(),
      lOutOffset, lOutPhyOffset);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  QI_LOCK(&mMutex);
  mError = QI_SUCCESS;
  if (mCropFlag) {
    lrc = mpDma->Start(*mInput, *mOutput, mCrop, true);
  } else {
    lrc = mpDma->Start(*mInput, *mOutput, true);
  }
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  if (mTimeoutFlag) {
    lrc = QIThread::WaitForCompletion(&mCond, &mMutex, mTimeout);
  }

  QI_UNLOCK(&mMutex);
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: print_usage
 *
 * Description: print the usage of the test application
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void print_usage()
{
  fprintf(stderr, "Usage: program_name [options] [-I <input file>]"
    " [-O <output file] [-W <width>] [-H <height>] [-F <format>]\n");
  fprintf(stderr, "Mandatory options:\n");
  fprintf(stderr, "  -I FILE\t\tPath to the input file.\n");
  fprintf(stderr, "  -O FILE\t\tPath for the output file.\n");
  fprintf(stderr, "  -W WIDTH\t\tInput image width.\n");
  fprintf(stderr, "  -H HEIGHT\t\tInput image height.\n");
  fprintf(stderr, "  -w WIDTH\t\tOutput image width.\n");
  fprintf(stderr, "  -h HEIGHT\t\tOutput image height.\n");
  fprintf(stderr, "  -u HEIGHT\t\tCrop enable flag.\n");
  fprintf(stderr, "  -x HEIGHT\t\tCrop left offset.\n");
  fprintf(stderr, "  -y HEIGHT\t\tCrop top offset.\n");
  fprintf(stderr, "  -p HEIGHT\t\tCrop image width.\n");
  fprintf(stderr, "  -q HEIGHT\t\tCrop image hight.\n");
  fprintf(stderr, "  -F FORMAT\t\tInput image format:\n");
  fprintf(stderr, "\t\t\t\t%s (0), %s (1), %s (2) %s (3)\n"
    "\t\t\t\t%s (4), %s (5), %s (6) %s (7)\n "
    "\t\t\t\t%s (8), %s (9), %s (10) %s (11),\n"
    "\t\t\t\t%s (12), %s (13), %s (14), %s (15), %s (16).\n",
    col_formats[0].format_str, col_formats[1].format_str,
    col_formats[2].format_str, col_formats[3].format_str,
    col_formats[4].format_str, col_formats[5].format_str,
    col_formats[6].format_str, col_formats[7].format_str,
    col_formats[8].format_str, col_formats[9].format_str,
    col_formats[10].format_str, col_formats[11].format_str,
    col_formats[12].format_str, col_formats[13].format_str,
    col_formats[14].format_str, col_formats[15].format_str,
    col_formats[16].format_str);
  fprintf(stderr, "  -T TIME\t\tAbort time in millisec.\n");
  fprintf(stderr, "\n");
}

/*===========================================================================
 * Function: main
 *
 * Description: main dma test app routine
 *
 * Input parameters:
 *   argc - argument count
 *   argv - argument strings
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int main(int argc, char* argv[])
{
  int rc, c, i;
  int lrc = QI_SUCCESS;
  QJpegDmaTest *lDma = new QJpegDmaTest();
  QITime lTime;
  uint64_t lDmaTime = 0LL;
  uint32_t lDmaLeftCrop = 0,
    lDmaTopCrop = 0,
    lDmaCropWidth = 0,
    lDmaCropHeight = 0,
    val = 0;

  fprintf(stderr, "=======================================================\n");
  fprintf(stderr, " QTI Dma test\n");
  fprintf(stderr, "=======================================================\n");
  opterr = 1;

  while ((c = getopt(argc, argv, "I:O:W:H:w:h:u:x:y:p:q:T:PZF:")) != -1) {
    switch (c) {
    case 'O':
      lDma->mOutputFilename = optarg;
      fprintf(stderr, "%-25s%s\n", "Output image path",
        lDma->mOutputFilename);
      break;
    case 'I':
      lDma->mInputFilename = optarg;
      fprintf(stderr, "%-25s%s\n", "Input image path",
        lDma->mInputFilename);
      break;
    case 'W':
      lDma->mSize.setWidth(atoi(optarg));
      fprintf(stderr, "%-25s%d\n", "Input image width",
        lDma->mSize.Width());
      break;
    case 'H':
      lDma->mSize.setHeight(atoi(optarg));
      fprintf(stderr, "%-25s%d\n", "Input image height",
        lDma->mSize.Height());
      break;
    case 'w':
      lDma->mOutSize.setWidth(atoi(optarg));
      fprintf(stderr, "%-25s%d\n", "Output image width",
          lDma->mOutSize.Width());
      break;
    case 'h':
      lDma->mOutSize.setHeight(atoi(optarg));
      fprintf(stderr, "%-25s%d\n", "Output image height",
          lDma->mOutSize.Height());
      break;
    case 'u':
      lDma->mCropFlag = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "DMA crop flag", lDma->mCropFlag);
      break;
    case 'x':
      lDmaLeftCrop = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Left Crop offset", lDmaLeftCrop);
      break;
    case 'y':
      lDmaTopCrop = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Top Crop offset", lDmaTopCrop);
      break;
    case 'p':
      lDmaCropWidth = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Crop image width", lDmaCropWidth);
      break;
    case 'q':
      lDmaCropHeight = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Crop image height", lDmaCropHeight);
      break;
    case 'F':
      val = atoi(optarg);
      CLAMP(val, 0, MAX_COLOR_FMTS - 1);
      lDma->mFormat = col_formats[val].eColorFormat;
      lDma->mSS = col_formats[val].ss;
      break;
    case 'T':
      lDma->mTimeout = atoi(optarg);
      lDma->mTimeoutFlag = 1;
      fprintf(stderr, "%-25s%d\n", "Abort time in millisec",
        lDma->mTimeout);
      break;
    default:
      break;
    }
  }

  if ((lDma->mSize.Width() & 1) || (lDma->mSize.Height() & 1) ||
      (lDma->mOutSize.Width() & 1) || (lDma->mOutSize.Height() & 1)) {
    delete lDma;
    fprintf(stderr, "%-25s\n", "Image sizes must be even!");
    return 1;
  }

  if ((lDma->mSize.Width() < lDma->mOutSize.Width()) ||
      (lDma->mSize.Height() < lDma->mOutSize.Height())) {
    delete lDma;
    fprintf(stderr, "%-25s\n", "Upscale not supported!");
    return 1;
  }

  if (((float)lDma->mSize.Height() / lDma->mOutSize.Height()) > MAX_DOWNSCALE) {
    delete lDma;
    fprintf(stderr, "%-25s %f\n", "Error, max downscale ratio is",
        MAX_DOWNSCALE);
    return 1;
  }

  if (lDma->mCropFlag) {
    lDma->mCrop.setCrop(lDmaLeftCrop, lDmaTopCrop,
      (lDmaLeftCrop + lDmaCropWidth),
      (lDmaTopCrop + lDmaCropHeight));

    if (lDma->mCrop.isValid() == false
      || lDma->mCrop.Width() > (lDma->mSize.Width() - lDma->mCrop.Left())
      || lDma->mCrop.Height() > (lDma->mSize.Height() - lDma->mCrop.Top())) {
      delete lDma;
      fprintf(stderr, "%-25s\n", "Invalid crop paramerters!");
      return 1;
    }

    if (lDma->mCrop.Width() < lDma->mOutSize.Width()
      || lDma->mCrop.Height() < lDma->mOutSize.Height()) {
      delete lDma;
      fprintf(stderr, "%-25s\n", "Crop and Upscale not supported!");
      return 1;
    }
  }

  if ((lDma->mOutputFilename != NULL) &&
    (lDma->mOutputFilename != NULL) &&
    !(lDma->mSize.IsZero())) {
    fprintf(stderr, "%-25s\n", "Dma started");
  } else {
    delete lDma;
    print_usage();
    return 1;
  }

  lrc = lDma->Init();
  if (lrc != QI_SUCCESS) {
    delete lDma;
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = lDma->Read();
  if (lrc != QI_SUCCESS) {
    delete lDma;
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  QIDBG_HIGH("%s:%d] Timer Start", __func__, __LINE__);
  lTime.Start();

  lrc = lDma->Start();
  if (lrc != QI_SUCCESS) {
    delete lDma;
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  QIDBG_HIGH("%s:%d] Timer End", __func__, __LINE__);
  lDmaTime = lTime.GetTimeInMilliSec();

  lrc = lDma->Write();
  if (lrc != QI_SUCCESS) {
    delete lDma;
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }
  fprintf(stderr, "%-25s%llu\n", "Dma completed in milliseconds ",
    lDmaTime);
  fprintf(stderr, "%-25s%d\n", "Dmad size ", lDma->mOutputSize);
  delete lDma;
  return 0;
}
