/*==============================================================================
 * Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

package com.qualcomm.qti.touch2track;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.util.List;

import com.qualcomm.qti.touch2track.R;
import com.qualcomm.qti.touch2track.util.ConditionalLog;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.ToggleButton;
import android.widget.Toast;

/** The main activity for the FastCV FAST Corner sample app. */
public class FastCVSample extends Activity
{
   /** Logging tag */
   private static final int         MAX_OBJS               = 3;

   /** Maximum number of frames to try RegisterObjByPoint */
   private static final int         MAXFRAMECNT               = 7;

   /** Logging tag */
   private static final String         TAG               = "FastCVSample";

   /** Num preview buffers */
   private static final int         NUMPREVIEWBUFFERS    = 4;

   /** Android camera object */
   private Camera                      mCamera;

   /** GL Surface view to render camera and corners via GPU */
   private GLSurfaceView               mGLSurface = null;

   /** Surface Holder for camera, Camera API requirement */
   private CameraSurface               mCamSurface = null;

   /** Root view group for all views associated with this
    *  activity */
   private ViewGroup                   mRootViewGroup = null;

   /** Java class associated with the camera and corner
    *  renderering */
   private FastCVSampleRenderer        mRenderer;

   /** Text boxes for on screen display of information */
   private TextView[]                  mText = new TextView[2];

   /** Android UI button to switch between start and stop processing */
   private ToggleButton                tbtnStartStop;

   /** Flag indicates whether or not preview is running. */
   private boolean                     mPreviewRunning   = false;

   /** Buffer for camera driver to store preview data, created
    *  statically so that it doesn't need to be re-allocated
    *  each frame. */
   private byte[]                      mPreviewBuffer    = null;

   /** Background color for text boxes. */
   private final int                   mBackgroundColor  = 0xFF728FCE; // ARGB
                                                                       //
   /** Text color for text boxes */
   private final int                   mTextColor        = 0xFFFFFFFF; // ARGB

   /** Filtered processing time   */
   private float                       mProcessTime      = 10;

   /** Width to try and configure camera preview */
   private static int                  mDesiredWidth     = 1920; //320; //320; //176;

   /** Height to try and configure camera preview */
   private static int                  mDesiredHeight    = 1080; //240; //240; //144;

   /** Actual width of camera preview image, can be different
  *  than desired if camera doesn't support desired width. */
   private int                         mPreviewWidth;

   /** Actual height of camera preview image, can be different
    *  than desired if camera doesn't support desired width. */
   private int                         mPreviewHeight;

   /** Actual width of camera screen. */
   private int                         mScreenWidth;

   /** Actual height of camera screen. */
   private int                         mScreenHeight;

   /** Screen/image ratio */
   private float[] mRatio  = new float[2];

   /** Preference options for threshold for Fast Corner Detector */
   private static final int[]          mPrefOptsFastThreshold = {5,10,20,40,80};

   /** Preference options for scaling for Fast Corner Detector */
   private static final int[]          mPrefOptsDownScale = {1,2,4,8,16};

   /**  Preference options for enabling/disabling viewfinder */
   private static final int[]          mPrefOptsViewFinder = {0, 1};

   // TODO: Why aren't these checks for booleans?
   /** Preference options for enabling/disabling corner overlay */
   private static final int[]          mPrefOptsEnableCornerOverlay = {0, 1};

   /** Preference options for enabling/disabling Gaussian Filter */
   private static final int[]          mPrefOptsEnableGaussianFilter = {0, 1};

   /** Handler for when to update profiling information. */
   private Handler                     mProfileHandler = new Handler();

   /** How long to wait between profiling updates. */
   private long                        mProfilePeriod = 1000;

   /** current tracking bounding box x position */
   private static int[] mCurrentTrackingBoudingX = new int[MAX_OBJS];

   /** current tracking bounding box y position */
   private static int[] mCurrentTrackingBoudingY =  new int[MAX_OBJS];

   /** tracking bounding box Left top X defined by touch event */
   private static int[]  mTrackingBoundingBoxX = new int[MAX_OBJS];

   /** tracking bounding box Left top Y defined by touch event */
   private static int[]  mTrackingBoundingBoxY = new int[MAX_OBJS];

   /** tracking bounding box Width defined by touch event */
   private static int[] mTrackingBoundingBoxWidth = new int[MAX_OBJS];

   /** tracking bounding box Height defined by touch event */
   private static int[]  mTrackingBoundingBoxHeight = new int[MAX_OBJS];

   /** draw bounding box Left top X defined by touch event */
   private static int  mDrawBoundingBoxX = 0;

   /** draw bounding box Left top Y defined by touch event */
   private static int  mDrawBoundingBoxY = 0;

   /** draw bounding box Width defined by touch event */
   private static int  mDrawBoundingBoxWidth = 0;

   /** draw bounding box Height defined by touch event */
   private static int mDrawBoundingBoxHeight = 0;

   /** draw bounding box aspect ratio */
   private static boolean bFixedAspectRation = false;
   private static float mDrawBoundingBoxAspectRatio = 1;

   /** flag for current touch */
   private boolean mTouchDownMove = false;

   /** last touch down time */
   private long lastTouchTimeInMs;

   /** touch down time for double-tap sensing*/
   private long doubleTapTimeinMs;

   /** tracker enable */
   private static boolean mTrackingEnable = false;
   private static boolean[] mTracking = new boolean[MAX_OBJS];

   /** unregister flag */
   private static boolean[] mUnregisterObject = new boolean[MAX_OBJS];

   /** location of unregister button on screen */
   private static int[] mUnregBoxLocX = new int[MAX_OBJS];
   private static int UNREGBOX_Y;
   private static int UNREGBOX_EDGE;

   /** for displaying touches on screen */
   private static int MAX_POINTS = 5;
   private static int POINT_TIMEOUT = 60;
   private static int numActivePoints;
   private static int mLastPointX, mLastPointY;
   private static int[] mPointTimeout = new int[MAX_POINTS];
   private static int[] mPointX = new int[MAX_POINTS];
   private static int[] mPointY = new int[MAX_POINTS];

   /** color of objects */
   private static int[] mObjColor = new int[MAX_OBJS];

   /** list of colors to be used */
   private final int[] COLORS = {Color.BLUE, Color.GREEN, Color.RED, Color.CYAN, Color.WHITE, Color.MAGENTA, Color.DKGRAY};

   /** tracking results **/
   private int[] mTrackingResult = new int[6*MAX_OBJS]; //upper left corner x, y, w, h, confidence, isZoomDisplayed

   /** object handles **/
   private int[] mIndex = new int[MAX_OBJS];

   /** object handles **/
   private int[] mIndexint = new int[MAX_OBJS];

   /** Preference options for use color input*/
   private static final boolean[]  bBoolean_opt = {false, true};

   /** display autozoom enable */
   private static boolean bEnableColorYUV = false; //true;

   private static final int[] nT2TModesAvailable = {0 /*Default*/, 1/*Performance*/, 2/*CPU-Offload*/,
                                                    3 /*Low-Power*/, 4/*Test-CPU*/, 5 /*Test-DSP*/};

   /** use DSP for tracking */
   private static int nT2TMode = 0;

   private static boolean bUseAsync = true;

   /** the last touch point x */
   private float[] mLastTouchX = new float[MAX_OBJS];

   /** the last touch point y */
   private float[] mLastTouchY = new float[MAX_OBJS];

   /** Number of objects currently selected by user */
   private int numActiveObjs = 0;

   /** flag to re-set the ROI for the tracker*/
   private boolean reset_ROI;

   /** for rotation **/
   private int mCameraOrientation;

   /** view to draw the rectangles and points  */
   FastCVSampleView mDrawView;

   /** Flag to determine if first frame in RegisterObjByPoint  */
   private static int[] newRoiFlag = new int[MAX_OBJS];
   /** Frame number for logic to implement RegisterObjByPoint  */
   private static int[] frameno = new int[MAX_OBJS];
   /** Frame number for logic to implement UnRegisterObjByPoint after 5 seconds*/
   private static int[] framenoforunregister = new int[MAX_OBJS];

   /** Time out after which the object will automatically be
    *  unregistered */
   final int TIMEOUT_IN_SECONDS = 604800; //== 7 days

   //for print screen
   private final String ROOT_SNAP_FOLDER = "/Touch2TrackSnaps";
   private boolean takePrints = true;
   private int maxScreenPrints = 0; // will control if print is to be taken or
   // no. Send 0 for no prints, or non zero
   // number for finite number of prints.
   private long lastPrintTimeNanoSec = 0;
   private final long PRINT_GAP_NANO_SEC = 300000000;
   private int printCounter = 0; // used if we need a fixed count of screen
   // count
   private String folderName = "snaps_def"; // default sub-folder name in case
   // name not sent
   // end print screen

   /** for Face detection */
   //private boolean mEnableFaceDetection = false;
   //private int numFaces=0;
   //private int[] mFaceRect = new int[200*4];

   static
   {
      // Load JNI library
      System.loadLibrary( "fastcvsample" );
   };

   /**
    * Camera callback to retrieve camera frames.
    *
    */
   private final Camera.PreviewCallback mCameraCallback =
   new Camera.PreviewCallback()
   {
      /**
        * Actual callback function for camera frames. Does per frame
        * processing.
        *
        * @param data buffer for preview data, in YUV420sp format.
        *  @param c Camera object.
        */
      public void onPreviewFrame( byte[] data, Camera c )
      {
         //Log.d("debug","here mTrackingBoundingBox:"+mTrackingBoundingBoxX + "mTrackingBoundingBoxY:"+mTrackingBoundingBoxY);

         //clean the drawing
         mDrawView.clean();

         // for text display on screen
         mDrawView.setResolution(mDesiredWidth+"x"+mDesiredHeight);
         int fps = (int)getCameraFPS();
         mDrawView.setFPS(fps+"");
         //end

         for(int i=0; i<numActivePoints; i++)
         {
            if(mPointTimeout[i] > 0)
            {
               mDrawView.addPoint(mPointX[i], mPointY[i], Color.WHITE);
               mPointTimeout[i]--;
            }
            else
            {
               deleteDisplayPoint(i);
            }
         }

         for(int i=0; i<numActiveObjs; )
         {
            if(mUnregisterObject[i] == true)
            {
               Log.i(TAG, "UNREGISTER OBJECT #" + i);

               unregisterROI(mIndexint[i]);

               for(int j = i+1; j < numActiveObjs; j++)
               {
                  mIndexint[j-1]=mIndexint[j];
                  mTrackingBoundingBoxWidth[j-1]=mTrackingBoundingBoxWidth[j];
                  mTrackingBoundingBoxHeight[j-1]=mTrackingBoundingBoxHeight[j];
                  mTrackingBoundingBoxX[j-1]=mTrackingBoundingBoxX[j];
                  mTrackingBoundingBoxY[j-1]=mTrackingBoundingBoxY[j];
                  mCurrentTrackingBoudingX[j-1] = mCurrentTrackingBoudingX[j];
                  mCurrentTrackingBoudingY[j-1] = mCurrentTrackingBoudingY[j];
                  mTracking[j-1] = mTracking[j];
                  framenoforunregister[j-1]=framenoforunregister[j];
                  mUnregisterObject[j-1] = mUnregisterObject[j];
                  mObjColor[j-1] = mObjColor[j];
               }

               numActiveObjs--;

               mIndexint[numActiveObjs]=0;
               mTrackingBoundingBoxWidth[numActiveObjs]=0;
               mTrackingBoundingBoxHeight[numActiveObjs]=0;
               mTrackingBoundingBoxX[numActiveObjs]=0;
               mTrackingBoundingBoxY[numActiveObjs]=0;
               mCurrentTrackingBoudingX[numActiveObjs] = 0;
               mCurrentTrackingBoudingY[numActiveObjs] = 0;
               mTracking[numActiveObjs] = false;
               framenoforunregister[numActiveObjs]=0;
               mUnregisterObject[numActiveObjs] = false;
               mObjColor[numActiveObjs] = Color.TRANSPARENT;
            }
            else
            {
               i++;
            }
         }

         for ( int i=0; i<numActiveObjs; i++ )
         {
            mDrawView.addRect(mUnregBoxLocX[i], UNREGBOX_Y, UNREGBOX_EDGE, UNREGBOX_EDGE, mObjColor[i]);

            if (bFixedAspectRation)
            {
               float ar = Math.abs((float)mTrackingBoundingBoxWidth[i]/(float)mTrackingBoundingBoxHeight[i]);
               if (ar < mDrawBoundingBoxAspectRatio)
                  mTrackingBoundingBoxHeight[i] = (int)(Math.abs(mTrackingBoundingBoxWidth[i]) / mDrawBoundingBoxAspectRatio) * (int)Math.signum(mTrackingBoundingBoxHeight[i]);
               else if (ar > mDrawBoundingBoxAspectRatio)
                  mTrackingBoundingBoxWidth[i] = (int)( Math.abs(mTrackingBoundingBoxHeight[i]) * mDrawBoundingBoxAspectRatio) * (int)Math.signum(mTrackingBoundingBoxWidth[i]);
            }

            if (mTrackingBoundingBoxWidth[i]<0)
            {
               mTrackingBoundingBoxX[i] = mTrackingBoundingBoxX[i] + mTrackingBoundingBoxWidth[i];
               mTrackingBoundingBoxWidth[i] = -mTrackingBoundingBoxWidth[i];
            }

            if (mTrackingBoundingBoxHeight[i]<0)
            {
               mTrackingBoundingBoxY[i] = mTrackingBoundingBoxY[i] + mTrackingBoundingBoxHeight[i];
               mTrackingBoundingBoxHeight[i] = -mTrackingBoundingBoxHeight[i];
            }

            if (bFixedAspectRation)
            {
               if (mTrackingBoundingBoxX[i] + mTrackingBoundingBoxWidth[i] >= mDesiredWidth)
               {
                  mTrackingBoundingBoxWidth[i] = mDesiredWidth - mTrackingBoundingBoxX[i] -1;
                  mTrackingBoundingBoxHeight[i] = (int)(mTrackingBoundingBoxWidth[i] / mDrawBoundingBoxAspectRatio);
               }
               if (mTrackingBoundingBoxY[i] + mTrackingBoundingBoxHeight[i] >= mDesiredHeight)
               {
                  mTrackingBoundingBoxHeight[i] = mDesiredHeight - mTrackingBoundingBoxY[i] -1;
                  mTrackingBoundingBoxWidth[i] = (int)( mTrackingBoundingBoxHeight[i] * mDrawBoundingBoxAspectRatio);
               }
            }

            if ((mCurrentTrackingBoudingX[i] != mTrackingBoundingBoxX[i] || mCurrentTrackingBoudingY[i] != mTrackingBoundingBoxY[i]))
            {
               //Register object by ROI
               if (mTrackingBoundingBoxWidth[i] > 20 && mTrackingBoundingBoxHeight[i] > 20)
               {
                  //if( mTrackingBoundingBoxY[i] > 10)
                  //    mTrackingBoundingBoxY[i] -= 10;

                  //Compensate for inverted camera

                  if (mCameraOrientation > 180)
                  {
                     mTrackingBoundingBoxX[i] = mPreviewWidth - mTrackingBoundingBoxWidth[i] - mTrackingBoundingBoxX[i];
                     mTrackingBoundingBoxY[i] = mPreviewHeight - mTrackingBoundingBoxHeight[i] - mTrackingBoundingBoxY[i];
                  }

                  //Set mTrackingEnable = true only if Registration is success
                  if (0==setROIext(data,mTrackingBoundingBoxX[i],mTrackingBoundingBoxY[i],mTrackingBoundingBoxWidth[i],mTrackingBoundingBoxHeight[i],mIndex))
                  {
                     mTrackingEnable = true;
                     mTracking[i]=true;
                     mIndexint[i]=mIndex[0];
                     mCurrentTrackingBoudingX[i] = mTrackingBoundingBoxX[i];
                     mCurrentTrackingBoudingY[i] = mTrackingBoundingBoxY[i];
                     Log.i(TAG,"mIndex[" + i + "]: " + mIndex[i]);
                     ConditionalLog.d(TAG, "Registration Successful, index:" + i +
                                      "= left:"   + mTrackingBoundingBoxX[i] +
                                      ", Top:"    + mTrackingBoundingBoxY[i] +
                                      ", Width:"  + mTrackingBoundingBoxWidth[i] +
                                      ", Height:" + mTrackingBoundingBoxHeight[i]);
                  }
                  else
                  {
                     numActiveObjs--;
                  }
                  //mTrackingEnable = true;
               }

               //Register object by Point
               else if (mTrackingBoundingBoxWidth[i] < 5 && mTrackingBoundingBoxHeight[i] < 5)
               {
                  //set flag to figure out if it is the first frame or not and keep a count
                  if (frameno[i]==0)
                     newRoiFlag[i] = 1;

                  //Compensate for inverted camera

                  if (mCameraOrientation > 180)
                  {
                     mTrackingBoundingBoxX[i] = mPreviewWidth - mTrackingBoundingBoxWidth[i] - mTrackingBoundingBoxX[i];
                     mTrackingBoundingBoxY[i] = mPreviewHeight - mTrackingBoundingBoxHeight[i] - mTrackingBoundingBoxY[i];
                  }

                  //Set mTrackingEnable = true only if Registration is success
                  Log.i(TAG,"mTrackingBoundingBoxX[i]: "+mTrackingBoundingBoxX[i]);
                  Log.i(TAG,"mTrackingBoundingBoxY[i]: "+mTrackingBoundingBoxY[i]);
                  if (0==setROIbyPointext(data,mTrackingBoundingBoxX[i],mTrackingBoundingBoxY[i],newRoiFlag[i],mIndex))
                  {
                     mTrackingEnable = true;
                     mTracking[i]=true;
                     frameno[i]=0;
                     mCurrentTrackingBoudingX[i] = mTrackingBoundingBoxX[i];
                     mCurrentTrackingBoudingY[i] = mTrackingBoundingBoxY[i];
                     mIndexint[i]=mIndex[0];
                     Log.i(TAG,"mIndex[" + i + "]: " + mIndex[i]);
                     ConditionalLog.d(TAG, "Registration Successful (point), index:" + i +
                                      "= X-coord:"   + mTrackingBoundingBoxX[i] +
                                      ", Y-coord:"    + mTrackingBoundingBoxY[i]);
                  }
                  else
                  {
                     frameno[i]++;
                     newRoiFlag[i] = 0;
                  }

                  if (frameno[i]>=MAXFRAMECNT)
                  {
                     numActiveObjs--;
                     frameno[i] = 0;
                  }
               }

               else
               {
                  numActiveObjs--;
               }
            }
         }//end of for loop for multiple objects

         // Increment FPS counter for camera.
         cameraFrameTick();

         // Perform processing on the camera preview data.
         if (mTrackingEnable && numActiveObjs>0)
         {
            mGLSurface = mRenderer.getGLSurfaceView();
            mGLSurface.setX(mScreenWidth - mGLSurface.getWidth());
            mGLSurface.setY(mScreenHeight - mGLSurface.getHeight());
            update( data, mTrackingResult);
         }

         for ( int i=0; i<numActiveObjs; i++ )
         {
            //Compensate for inverted camera
            if (mCameraOrientation > 180)
            {
               //x co-ord
               mTrackingResult[(i*6)+0] = mPreviewWidth - (mTrackingResult[(i*6)+0] + mTrackingResult[(i*6)+2]);
               //y co-ord
               mTrackingResult[(i*6)+1] = mPreviewHeight - (mTrackingResult[(i*6)+1] + mTrackingResult[(i*6)+3]);
            }

            ConditionalLog.d(TAG, "Bounding rect for Object" + i + "= left:"
                                + mTrackingResult[(i*6)] + ", Top:"
                                + mTrackingResult[(i*6)+1] + ", Width:"
                                + (mTrackingResult[(i*6)+2]) + ", Height:"
                                + (mTrackingResult[(i*6)+3]));// for logging

            if (mTrackingResult[(i*6)+2]>0 && mTrackingResult[(i*6)+3]>0)
            {
               if (mTracking[i]==true)
               {
                  framenoforunregister[i]++;
                  if (framenoforunregister[i]>(TIMEOUT_IN_SECONDS*30))
                  {
                     mUnregisterObject[i] = true;
                  }
                  mDrawView.addRect((int)(mTrackingResult[(i*6)]/mRatio[0]), (int)(mTrackingResult[(i*6)+1]/mRatio[1]),
                                    (int)(mTrackingResult[(i*6)+2]/mRatio[0]), (int)(mTrackingResult[(i*6)+3]/mRatio[1]),
                                     mObjColor[i]);
               }

               if (mTrackingResult[5]>0)
               {
                  mGLSurface.setVisibility(View.VISIBLE);
                  // Mark dirty for render.
                  requestRender();
               }
               else
                  mGLSurface.setVisibility(View.INVISIBLE);
            }
            else
            {
               mGLSurface = mRenderer.getGLSurfaceView();
               mGLSurface.setVisibility( View.GONE );
            }

         }//End of for loop over multiple objects

         if(mTrackingEnable && numActiveObjs>0)
         {
            //for print screen
            long time = System.nanoTime();
            //do printing only if max print counter is greater than 0
            if (time - lastPrintTimeNanoSec >= PRINT_GAP_NANO_SEC && printCounter < maxScreenPrints)
            {
               lastPrintTimeNanoSec = time;
               //from the raw byte array, create yuvimage, then compress to jpeg
               YuvImage yuvImg = new YuvImage(data, ImageFormat.NV21, mDesiredWidth, mDesiredHeight, null);
               ByteArrayOutputStream baos = new ByteArrayOutputStream();
               yuvImg.compressToJpeg(new Rect(0, 0, mDesiredWidth,   mDesiredHeight), 80, baos);
               //create required folders
               String root = Environment.getExternalStorageDirectory().toString();
               File myDir = new File(root + ROOT_SNAP_FOLDER);
               if (!myDir.exists())
               {
                  myDir.mkdirs();
               }

               myDir = new File(root + ROOT_SNAP_FOLDER + "/" + folderName);
               if (!myDir.exists())
               {
                  myDir.mkdirs();
               }

               //filename format for the first object is cntr_prevWidth_prevHeight_boxX_boxY_boxWidth_boxHeight.jpg
               String fname = printCounter + "_" + mDesiredWidth + "_"
                   + mDesiredHeight + "_" + mTrackingResult[0]
                   + "_" + mTrackingResult[1] + "_"
                   + mTrackingResult[2] + "_" + mTrackingResult[3]
                   + ".jpg";
               File file = new File(myDir, fname);
               if (file.exists())
                   file.delete();
               try
               {
                  FileOutputStream out = new FileOutputStream(file);
                  baos.writeTo(out);
                  out.flush();
                  out.close();
               }
               catch (Exception e)
               {
                  e.printStackTrace();
               }

               time = System.nanoTime() - time;
               printCounter++;
               Log.e(TAG, "Time taken " + time + " Currenttime "
                     + System.currentTimeMillis());
            }
         }
         //draw the touched box
         if (mTouchDownMove)
         {
            if (bFixedAspectRation)
            {
               float ar = Math.abs((float)mDrawBoundingBoxWidth/(float)mDrawBoundingBoxHeight);
               if (ar < mDrawBoundingBoxAspectRatio)
                  mDrawBoundingBoxHeight = (int)(Math.abs(mDrawBoundingBoxWidth) / mDrawBoundingBoxAspectRatio)*(int)Math.signum(mDrawBoundingBoxHeight);
               else if (ar > mDrawBoundingBoxAspectRatio)
                  mDrawBoundingBoxWidth = (int)( Math.abs(mDrawBoundingBoxHeight) * mDrawBoundingBoxAspectRatio)* (int)Math.signum(mDrawBoundingBoxWidth);
            }

            if (mDrawBoundingBoxWidth < 0)
            {
               mDrawBoundingBoxX = mDrawBoundingBoxX + mDrawBoundingBoxWidth;
               mDrawBoundingBoxWidth = -mDrawBoundingBoxWidth;
            }

            if (mDrawBoundingBoxHeight < 0)
            {
               mDrawBoundingBoxY = mDrawBoundingBoxY + mDrawBoundingBoxHeight;
               mDrawBoundingBoxHeight = -mDrawBoundingBoxHeight;
            }

            if (bFixedAspectRation)
            {
               if (mDrawBoundingBoxX + mDrawBoundingBoxWidth >= mDesiredWidth)
               {
                  mDrawBoundingBoxWidth = mDesiredWidth - mDrawBoundingBoxX -1;
                  mDrawBoundingBoxHeight = (int)(mDrawBoundingBoxWidth / mDrawBoundingBoxAspectRatio);
               }

               if (mDrawBoundingBoxY + mDrawBoundingBoxHeight >= mDesiredHeight)
               {
                  mDrawBoundingBoxHeight = mDesiredHeight - mDrawBoundingBoxY -1;
                  mDrawBoundingBoxWidth = (int)( mDrawBoundingBoxHeight * mDrawBoundingBoxAspectRatio);
               }
            }

            //draw the sliding box
            if (mDrawBoundingBoxWidth > 0 && mDrawBoundingBoxHeight > 0)
            {
               mDrawView.addRect((int)(mDrawBoundingBoxX/mRatio[0]), (int)(mDrawBoundingBoxY/mRatio[1]),
                                 (int)(mDrawBoundingBoxWidth/mRatio[0]),(int)(mDrawBoundingBoxHeight/mRatio[1]), Color.YELLOW);
            }
         }
         mDrawView.invalidate();

         // Simple IIR filter on time.
         mProcessTime = getFastCVProcessTime();

         if ( c != null )
         {
            // with buffer requires addbuffer each callback frame.
            c.addCallbackBuffer( data );
         }
      }
   };

   /**
    * Handler for FPS display. Want to do this periodically to not
    * waste CPU.
    */
   private Runnable mUpdateTimeTask = new Runnable()
   {
      /** Overridden run function to handle callback This updates
       *  the fps and number of corners detected for the user */
      public void run()
      {
         float camFPS = getCameraFPS();
         String message;

         Log.d(TAG, "T2T FPS " + camFPS);

         // Setup next profiling run.
         mProfileHandler.postDelayed( this, mProfilePeriod );

         message = String.format( "%.3fCamFPS, ", camFPS );
         //mText[0].setText( message + "Corners detected: " + numCorners );
         mText[ 1 ].setText(String.format( "Preview process time: %.2fms", mProcessTime ) );

         // Only add bg to utilized text boxes for now.
         for ( int i=0; i<mText.length; i++ )
         {
            if ( mText[i].getText().length() > 0 )
            {
               mText[i].setBackgroundColor( mBackgroundColor );
            }
            else
            {
               mText[i].setBackgroundColor( 0x00 );
            }
         }
      }
   };

   /**
    * Called when the activity is first created.
    */
   @Override
   public void onCreate(Bundle savedInstanceState)
   {
      super.onCreate( savedInstanceState );
      this.requestWindowFeature(Window.FEATURE_NO_TITLE);
      reset_ROI = true;
   }

   @Override
   public void onDestroy()
   {
      super.onDestroy();
      //FaceDeinit();
   }

   /**
    * Resumes camera preview
    */
   @Override
   protected void onResume()
   {
      super.onResume();
      int         screenw;
      int         width;

      //set to reset the ROIs
      reset_ROI = true;

      // Initialize UI
      setContentView( R.layout.main );

      mScreenWidth = ((WindowManager)this.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getWidth();
      mScreenHeight = ((WindowManager)this.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getHeight();

      mText[0] = (TextView) findViewById(R.id.text1);
      mText[1] = (TextView) findViewById(R.id.text2);

      mRootViewGroup = (ViewGroup)mText[0].getParent();

      width = (mScreenWidth * 1 / 2 );

      for ( int i=0; i<mText.length; i++ )
      {
         mText[i].setTextColor( mTextColor );
         mText[i].setWidth( width );
         mText[i].setText( "" );
      }

      mText[0].setText( "" );
      updatePreferences();

      tbtnStartStop = (ToggleButton) findViewById(R.id.startStopSwitch);
      tbtnStartStop.setOnClickListener(new View.OnClickListener()
                                       {
                                          public void onClick( View view )
                                          {
                                             if (tbtnStartStop.isChecked())
                                             {
                                                startPreview();
                                             }
                                             else
                                             {
                                                stopPreview();
                                             }
                                          }
                                       }
                                      );

      mCamera = Camera.open();

      // Camera surface is required for Camera to send
      // preview frame callback on some devices.
      // The order this is added to the rootViewGroup matters to ensure it is
      // below GLSurface. GLSurface is actually used for rendering.
      // Further note, when splash screen is not used this should be added
      // after GLSurfaceView. Really tricky to get ordering correct.

      mCamSurface = new CameraSurface( this, mCamera );
      mRootViewGroup.addView( mCamSurface );

      // Initialize the renderer
      mRenderer = new FastCVSampleRenderer( this );
      mGLSurface = mRenderer.getGLSurfaceView();

      //set the size of the view
      LayoutParams params = new LayoutParams(240, 240); //(int)(mScreenHeight/3), (int)(mScreenHeight/3));
      mGLSurface.setLayoutParams(params);
      mGLSurface.setZOrderMediaOverlay(true);
      //set the location of the view
      //mGLSurface.setX(mScreenWidth - mGLSurface.getWidth());
      //mGLSurface.setY(mScreenHeight - mGLSurface.getHeight());
      mGLSurface.setVisibility( View.INVISIBLE );
      mGLSurface.setRenderMode( GLSurfaceView.RENDERMODE_WHEN_DIRTY );
      mGLSurface.setLayerType(View.LAYER_TYPE_HARDWARE, null);
      mRootViewGroup.addView( mGLSurface );

      //add the draw view
      mDrawView=new FastCVSampleView(this);
      mDrawView.setVisibility(View.VISIBLE);
      mRootViewGroup.addView(mDrawView);

      //set orientation
      {
         android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();
         if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT)
            android.hardware.Camera.getCameraInfo(1, info);
         else
            android.hardware.Camera.getCameraInfo(0, info);

         int rotation = getWindowManager().getDefaultDisplay().getRotation();
         int degrees = 0;

         switch (rotation)
         {
         case android.view.Surface.ROTATION_0:   degrees = 0;   break;
         case android.view.Surface.ROTATION_90:  degrees = 90;  break;
         case android.view.Surface.ROTATION_180: degrees = 180; break;
         case android.view.Surface.ROTATION_270: degrees = 270; break;
         }

         mCameraOrientation = info.orientation;

         int result;
         if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT)
         {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;  // compensate the mirror
         }
         else  // back-facing
         {
            result = (info.orientation - degrees + 360) % 360;
         }
         mCamera.setDisplayOrientation(result);
      }
      bringControlsToFront();
      startPreview();
      System.gc();
   }

   /**
    * Retrieves shared preferences and places them in member
    * variables for use.
    */
   protected void updatePreferences()
   {
      // Retrieve Preferences
      SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences( getBaseContext() );

      // for preview resolution, for print screen
      Bundle extras = this.getIntent().getExtras();
      int prevIndx = 0;
      int width = mDesiredWidth;
      int height = mDesiredHeight;
      try
      {
         if (extras != null && extras.containsKey("resolution"))
         {
            StringBuilder val = new StringBuilder(
                                                 extras.getString("resolution"));
            Log.d(TAG, "from intent: " + val);
            width = Integer.parseInt(val.substring(0, val.indexOf("_")));
            height = Integer.parseInt(val.substring(val.indexOf("_") + 1));
         }
         else
         {
            Log.e(TAG, "No intent param");
            String val = prefs.getString("resolution", mDesiredWidth + "_"
                                         + mDesiredHeight);
            Log.d(TAG, "from prefs: " + val);
            width = Integer.parseInt(val.substring(0, val.indexOf("_")));
            height = Integer.parseInt(val.substring(val.indexOf("_") + 1));
         }
      }
      catch (Exception e)
      {
         width = mDesiredWidth;
         height = mDesiredHeight;
      }
      mDesiredWidth = width;
      mDesiredHeight = height;
      try
      {
         if (extras != null && extras.containsKey("max_prints"))
         {
            String val = extras.getString("max_prints");
            Log.d(TAG, "Max prints from intent: " + val);
            maxScreenPrints = Integer.parseInt(val);
         }
      }
      catch (Exception e)
      {
         maxScreenPrints = 0;
      }
      try
      {
         if (extras != null && extras.containsKey("folder_name"))
         {
            folderName = extras.getString("folder_name");
            Log.d(TAG, "Folder Name from intent: " + folderName);
         }
      }
      catch (Exception e)
      {
      }
      // if folder by the given name exists, append a timestamp to the new foldername
      String root = Environment.getExternalStorageDirectory().toString();
      File dir = new File(root + ROOT_SNAP_FOLDER);
      if (dir.exists())
      {
         File myDir = new File(dir, folderName);
         if (myDir.exists())
         {
            Log.e(TAG, folderName + " already exists.");
            folderName = folderName + "_" + System.currentTimeMillis();
            Log.e(TAG, "changing the folder name to " + folderName);
         }
      }
      // end print screen

      int resIndex;

      //commandline for t2t-mode
      try
      {
         if (extras != null && extras.containsKey("T2TMode"))
         {
            String val = extras.getString("T2TMode");
            Log.d(TAG, "T2T Mode from intent: " + val);
            resIndex = Integer.parseInt(val);
            if (resIndex > 1)
               resIndex = 1;
         }
         else
         {
            resIndex = Integer.decode(prefs.getString("T2TMode", "0"));
         }
      }
      catch (Exception e)
      {
         e.printStackTrace();
         resIndex = 0;
      }

      if ( resIndex < nT2TModesAvailable.length )
      {
         nT2TMode = nT2TModesAvailable[ resIndex ];
      }
      Log.e( TAG, "Configured T2T Mode " + nT2TMode );
      //end commandline dsp


      resIndex = Integer.decode( prefs.getString( "APIType", "1" ) );
      if ( resIndex < bBoolean_opt.length )
      {
         bUseAsync = bBoolean_opt[ resIndex ];
      }

      bUseAsync = false;

      if(bUseAsync)
      {
         Log.e( TAG, "Configured API Type : Async");
      }
      else
      {
         Log.e( TAG, "Configured API Type : Sync");
      }

      resIndex = Integer.decode( prefs.getString( "FixedAspectRatioEnable", "0" ) );
      if ( resIndex < bBoolean_opt.length )
      {
         bFixedAspectRation = bBoolean_opt[ resIndex ];
      }
      Log.e( TAG, "Configured Fixed Aspect Ratio: " + bFixedAspectRation );

      resIndex = Integer.decode( prefs.getString( "ColorYUVEnable", "1" ) );
      if ( resIndex < bBoolean_opt.length )
      {
         bEnableColorYUV = bBoolean_opt[ resIndex ];
      }
      Log.e( TAG, "Configured Color YUV for tracking: " + bEnableColorYUV );
   }

   /**
    * Brings the controls to the foreground
    */
   protected void bringControlsToFront()
   {
      for ( int i=0; i<mText.length; i++ )
      {
         mText[i].bringToFront();
      }
      tbtnStartStop.bringToFront();
   }

   /**
    * Suspends camera preview, cleans up the renderer
    */
   @Override
   protected void onPause()
   {
      super.onPause();
      stopPreview();
      if ( mCamera != null )
      {
         mCamera.release();
         mCamera = null;
      }

      if ( mRenderer != null )
      {
         mRenderer.cleanup();
      }

      if ( mDrawView != null )
      {
         mDrawView.invalidate();
      }

      if ( mRootViewGroup != null )
      {
         mRootViewGroup.removeAllViews();
      }

      cleanup();
      //cleanupFacedetect();

      //set to reset the ROIs
      reset_ROI = true;

      numActiveObjs=0;
      // Do a full exit here. On some devices the views won't return
      // in proper order. Since there is no state, just exit the activity
      // and start over on next launch.
      // System.exit(0);
   }
   /**
   * Creates the options menu
   * @param menu
   */
   @Override
   public boolean onCreateOptionsMenu( Menu menu )
   {
      MenuInflater inflater = getMenuInflater();
      inflater.inflate( R.menu.menu, menu );
      return true;
   }

   /**
    * User Option selection menu
    * @param item The menu item
    */
   @Override
   public boolean onOptionsItemSelected( MenuItem item )
   {
      // Handle item selection
      switch ( item.getItemId() )
      {
      case R.id.settings:
         Intent settingsActivity =
         new Intent( getBaseContext(), Preferences.class );
         startActivity( settingsActivity );
         return true;

      case R.id.about:
         Intent aboutActivity =
         new Intent( getBaseContext(), About.class );
         startActivity( aboutActivity );
         return true;

      default:
         return super.onOptionsItemSelected(item);
      }
   }

   /**
    * Sets up  camera configuration parameters
    */
   private void setupCamera()
   {
      /* By default the camera preview size will be the highest supported resolution.
        * * In case one of the supported resolutions has a height of 480, this will always
        * * be used in conjunction with the corresponding width i.e. a WVGA or size closest
        * to this (upto 480 height) will be supported.
        * To support higher resolutions e.g. on tablets a user configurable setting can
        * be provided at a later stage.
        */
      Camera.Parameters parameters = mCamera.getParameters();
      //List<Camera.Size> sizes = mCamera.getParameters().getSupportedPreviewSizes();
      //Camera.Size list = sizes.get(0);
      //mDesiredWidth = list.width;
      //mDesiredHeight = list.height;
      /*
      for (Camera.Size s: sizes) {
            if (s.height == 480) //240 //144
            {
                mDesiredWidth = s.width;
                mDesiredHeight = s.height;
                break;
            }
      }
      */

      // Now that the size is known, set up the camera parameters and begin
      // the preview.
      int k=0;
      List<int[]> supportedFps = parameters.getSupportedPreviewFpsRange();
      int[] maxFps = supportedFps.get(supportedFps.size()-1);
      parameters.setPreviewFpsRange(maxFps[0], maxFps[1]);
      Log.v (TAG, "Set FPS Range - (" + maxFps[0] + "," + maxFps[1] + ")");

      parameters.setPreviewSize( mDesiredWidth, mDesiredHeight );

      //
      //  Set Focus mode depending on what is supported. MODE_INFINITY is
      //  preferred mode.
      //
      List<String> supportedFocusModes = parameters.getSupportedFocusModes();
      if ( supportedFocusModes!= null )
      {
         if ( supportedFocusModes.contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO) )
         {
            parameters.setFocusMode( Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
            Log.v( TAG, "Set focus mode FOCUS_MODE_CONTINUOUS_VIDEO" );
         }
         else if ( supportedFocusModes.contains(Camera.Parameters.FOCUS_MODE_INFINITY) )
         {
            parameters.setFocusMode( Camera.Parameters.FOCUS_MODE_INFINITY );
            Log.v( TAG, "Set focus mode INFINITY" );
         }
         else if ( supportedFocusModes.contains(Camera.Parameters.FOCUS_MODE_FIXED) )
         {
            parameters.setFocusMode( Camera.Parameters.FOCUS_MODE_FIXED );
            Log.v( TAG, "Set focus mode FIXED" );
         }
      }

      //
      // Set White Balance to Auto if supported.
      //
      List<String> supportedWhiteBalance = parameters.getSupportedWhiteBalance();
      if ( supportedWhiteBalance != null && supportedWhiteBalance.contains (Camera.Parameters.WHITE_BALANCE_AUTO) )
      {
         parameters.setWhiteBalance( Camera.Parameters.WHITE_BALANCE_AUTO );
         Log.v( TAG, "Set white balance AUTO" );
      }
      try
      {
         mCamera.setParameters( parameters );
      }
      catch ( RuntimeException re )
      {
         // NOTE that we shouldn't be here as we check where our specified
         // parameters are supported or not.
         //
         re.printStackTrace();
         Log.e( TAG,"Unable to set Camera Parameters" );
         Log.i( TAG,"Falling back to setting just the camera preview" );
         parameters = mCamera.getParameters();
         Log.d(TAG, "Preview size: " + mDesiredWidth + "x" + mDesiredHeight);
         parameters.setPreviewSize( mDesiredWidth, mDesiredHeight );
         try
         {
            mCamera.setParameters( parameters );
         }
         catch ( RuntimeException re2 )
         {
            re2.printStackTrace();
            Log.e( TAG, "Problem with camera configuration, unable to set "+ "Camera Parameters. Camera not available." );
         }
      }

      mPreviewWidth  = mCamera.getParameters().getPreviewSize().width;
      mPreviewHeight = mCamera.getParameters().getPreviewSize().height;

      UNREGBOX_EDGE = (int) (mScreenHeight * 0.085);
      UNREGBOX_Y    = (int) (mScreenHeight * 0.11);
      mUnregBoxLocX[0] = (int) (UNREGBOX_EDGE*0.1);
      for(int i=1; i<MAX_OBJS; i++)
      {
         mUnregBoxLocX[i] = mUnregBoxLocX[i-1] + (int) (UNREGBOX_EDGE*1.1);
      }

      mRatio[0] = (float)mPreviewWidth/(float)mScreenWidth;
      mRatio[1] = (float)mPreviewHeight/(float)mScreenHeight;

      Log.d(TAG, "Preview size: " + mPreviewWidth + "x" + mPreviewHeight);
      Log.d(TAG, "Screen  size: " + mScreenWidth + "x" + mScreenHeight);

      //initialize the trackers in fastcv.
      init(mPreviewWidth,mPreviewHeight, bEnableColorYUV, nT2TMode, bUseAsync);

      //initialize the face detector
      //FaceInit( ); //mPreviewWidth, mPreviewHeight);
      //setScalingFacedetect(1);
      //setDetectionMode(0);

      mTrackingEnable = false;
      mDrawBoundingBoxX = 0;
      mDrawBoundingBoxY = 0;
      mDrawBoundingBoxHeight = 0;
      mDrawBoundingBoxWidth = 0;

      for (int i=0;i<MAX_OBJS;i++)
      {
         mCurrentTrackingBoudingX[i]=0;
         mCurrentTrackingBoudingY[i]=0;
         mTrackingBoundingBoxX[i]=0;
         mTrackingBoundingBoxY[i]=0;
         mTrackingBoundingBoxWidth[i]=0;
         mTrackingBoundingBoxHeight[i]=0;
         frameno[i]=0;
         framenoforunregister[i]=0;
         newRoiFlag[i]=0;
         mTracking[i]=false;
         mUnregisterObject[i] = false;
      }
      numActiveObjs=0;
   }

   /**
    * Starts camera preview
    */
   private void startPreview()
   {
      reset_ROI = true;

      if ( mCamera != null )
      {
         setupCamera();
         setCallback();
         mCamera.startPreview();
         mPreviewRunning = true;
      }
      else
      {
         mPreviewRunning = false;
      }

      mProfileHandler.removeCallbacks( mUpdateTimeTask );
      mProfileHandler.postDelayed( mUpdateTimeTask, mProfilePeriod );
   }

   /**
    * Stops camera preview
    */
   private void stopPreview()
   {
      if ((mCamera != null) && (mPreviewRunning == true))
      {
         mCamera.stopPreview();
         mCamera.setPreviewCallback( null );
      }

      mProfileHandler.removeCallbacks( mUpdateTimeTask );
      mPreviewRunning = false;
      System.gc();
   }

   /**
    * Sets the Camera preview callback
    *
    */
   public void setCallback()
   {
      int bufferSize = 0;
      int pformat;
      int bitsPerPixel;

      Camera.Parameters p = mCamera.getParameters();
      //List<Integer> formats = p.getSupportedPreviewFormats();
      //for(int i=0;i<formats.size();i++){Log.d(TAG, "formats.get(i) = " + formats.get(i));}

      pformat = p.getPreviewFormat();

      if(pformat != ImageFormat.NV21)
      {
         p.setPreviewFormat(ImageFormat.NV21);
         mCamera.setParameters(p);
      }

      // Get pixel format information to compute buffer size.
      PixelFormat info = new PixelFormat();
      PixelFormat.getPixelFormatInfo( pformat, info );
      bitsPerPixel = info.bitsPerPixel;

      bufferSize = mPreviewWidth * mPreviewHeight * bitsPerPixel / 8;

      // Make sure buffer is deleted before creating a new one.
      mPreviewBuffer = null;

      for(int i=0; i<NUMPREVIEWBUFFERS; i++)
      {
         // New preview buffer.
         mPreviewBuffer = new byte[ bufferSize + 4096 ];

         // with buffer requires addbuffer.
         mCamera.addCallbackBuffer( mPreviewBuffer );
      }

      mCamera.setPreviewCallbackWithBuffer( mCameraCallback );
   }

   @Override
   public boolean onTouchEvent(MotionEvent ev)
   {
      final int action = ev.getAction();
      boolean intercept = true;
      int nUnregX = -1;
      int nUnregY = -1;

      if(numActiveObjs < MAX_OBJS) {
         nUnregX = (mUnregBoxLocX[numActiveObjs] + UNREGBOX_EDGE + 1);
         nUnregY = (UNREGBOX_Y + 1);
      }

      switch (action)
      {
      case MotionEvent.ACTION_DOWN:
         {
            final float x = ev.getX();
            final float y = ev.getY();
            Log.i(TAG,"down x: "+x);
            Log.i(TAG,"down y: "+y);

            lastTouchTimeInMs = System.currentTimeMillis();

            if (numActiveObjs<MAX_OBJS)
            {
               if ((int)(((int)(x*mRatio[0]))/mRatio[0]) > nUnregX ||
                   (int)(((int)(y*mRatio[1]))/mRatio[1]) > nUnregY )
               {
                  mLastTouchX[numActiveObjs] = x;
                  mLastTouchY[numActiveObjs] = y;
               }
            }
            break;
         }

      case MotionEvent.ACTION_MOVE:
         {
            final float x = ev.getX();
            final float y = ev.getY();
            if (numActiveObjs<MAX_OBJS)
            {
               if ((int)(((int)(x*mRatio[0]))/mRatio[0]) > nUnregX ||
                   (int)(((int)(y*mRatio[1]))/mRatio[1]) > nUnregY )
               {
                  mTouchDownMove = true;
                  mDrawBoundingBoxX = (int) (mLastTouchX[numActiveObjs] * mRatio[0]);
                  mDrawBoundingBoxY = (int) (mLastTouchY[numActiveObjs] * mRatio[1]);
                  if (mDrawBoundingBoxY>10)
                     mDrawBoundingBoxY -= 10;
                  mDrawBoundingBoxWidth = (int) ((x - mLastTouchX[numActiveObjs])*mRatio[0]);
                  mDrawBoundingBoxHeight = (int) ((y - mLastTouchY[numActiveObjs])*mRatio[1]);

                  //Check for this only if it is the first object tracked
                  if (numActiveObjs==0)
                  {
                     long currTime = System.currentTimeMillis();
                     if (currTime - lastTouchTimeInMs > 200)
                        mTrackingEnable = false;
                  }
               }
            }
            break;
         }

      case MotionEvent.ACTION_UP:
         {
            doubleTapTimeinMs = System.currentTimeMillis();
            final float x = ev.getX();
            final float y = ev.getY();
            Log.i(TAG,"Up x: "+x);
            Log.i(TAG,"Up y: "+y);
            boolean bTouchUnregister = false;

            if(numActiveObjs<MAX_OBJS)
            {
               if(((x - mLastTouchX[numActiveObjs])*mRatio[0]) < 10)
               {
                  if(((y - mLastTouchY[numActiveObjs])*mRatio[1]) < 10)
                  {
                     addDisplayPoint((int)x,(int)y);
                  }
               }
            }

            if ((doubleTapTimeinMs-lastTouchTimeInMs)>5 && numActiveObjs>0)
            {
               for(int i=0; i<numActiveObjs; i++)
               {
                  int ty = (int)(((int)(y*mRatio[1]))/mRatio[1]);
                  if(ty >= UNREGBOX_Y && ty <= UNREGBOX_Y + UNREGBOX_EDGE)
                  {
                     int tx = (int)(((int)(x*mRatio[0]))/mRatio[0]);
                     if (tx >= mUnregBoxLocX[i] && tx <= mUnregBoxLocX[i] + UNREGBOX_EDGE)
                     {
                        mUnregisterObject[i] = true;
                        bTouchUnregister = true;
                        break;
                     }
                  }
               }
            }

            if (bTouchUnregister == false && numActiveObjs<MAX_OBJS)
            {
               if ((int)(((int)(x*mRatio[0]))/mRatio[0]) > nUnregX ||
                   (int)(((int)(y*mRatio[1]))/mRatio[1]) > nUnregY )
               {
                  mTouchDownMove = false;
                  int t = (int) (mLastTouchX[numActiveObjs] * mRatio[0]);
                  mTrackingBoundingBoxX[numActiveObjs] = t;
                  mTrackingBoundingBoxY[numActiveObjs] = (int) (mLastTouchY[numActiveObjs] * mRatio[1]);
                  mTrackingBoundingBoxWidth[numActiveObjs] = (int) ((x - mLastTouchX[numActiveObjs])*mRatio[0]);
                  mTrackingBoundingBoxHeight[numActiveObjs] = (int) ((y - mLastTouchY[numActiveObjs])*mRatio[1]);
                  mObjColor[numActiveObjs] = 0;

                  for(int i=0; i<COLORS.length; i++) {
                     int j=0;
                     for( ; j<numActiveObjs; j++) {
                        if(mObjColor[j] == COLORS[i]) {
                           break;
                        }
                     }
                     if(j==numActiveObjs) {
                        mObjColor[numActiveObjs] = COLORS[i];
                        break;
                     }
                  }

                  numActiveObjs++;
               }
            }
            break;
         }
      }
      return intercept;
   }

   /** DO-NOT modify this variable from JAVA.
    *  This holds the handle to native context instance
    *  associated with this instance of the java class */
   private long   mNativeContext = 0;

   /** Native function declarations*/
   public native float getCameraFPS();

   /**
    * Initializes native processing
    */
   public native void init(int w, int h, boolean enableColorYUV, int trackerMode,
                           boolean runAsync);

   /**
    * Increments camera frame counter to track FPS.
    *
    */
   private native void cameraFrameTick();

   /**
    * Function to pass camera frame for native, FastCV processing.
    * @param data Byte buffer for data.
    * @param w Width of data
    * @param h Height of data
    */

   public native void update ( byte[] data, int[] trackingResult );

   /**
    * Function to pass camera frame for region of interest
    * selection that returns the ObjectIndex.
    * @param data Byte buffer for data.
    * @param x, left top x of bouding box
    * @param y, left top y of bounding box
    * @param w Width of bounding box
    * @param h Height of bounding box
    * @param pIndex Index of the registered object
    */
   public native int setROIext( byte[] data, int x, int y,int w, int h, int[] pIndex);

   /**
    * Function to pass camera frame for region of interest
    * selection that returns the ObjectIndex.
    * @param data Byte buffer for data.
    * @param x, left top x of bouding box
    * @param y, left top y of bounding box
    * @param newROIFlag flag to specify if it is the first attempt at registering object by point
    * @param h Height of bounding box
    * @param pIndex Index of the registered object
    */
   public native int setROIbyPointext( byte[] data, int x, int y, int newROIFlag, int[] pIndex);

   /**
    * Function to pass camera frame for region of interest
    * unregisteration using ObjectIndex.
    * @param index Index of object to be unregistered.
    */
   public native int unregisterROI( int index);

   /** Performs native cleanup routines for application exit. */
   private native void cleanup();

   /**
    * Retrieves FastCV processing timing from native layer.
    *
    * @return FastCV processing timing, filtered, in ms.
    */
   private native float getFastCVProcessTime();


   /*************************************************************************/
   /** Face Detection **/
   //private native void FaceInit(); //(int w, int h);
   //private native void FaceDeinit();

   /**
    * Sets the desired scaling factor.
    *
    *
    * @param scaleFactor Desired scaling factor, 1 is no scaling.
    */
   //private native void setScalingFacedetect(int scaleFactor);
   //private native void setDetectionMode(int detectionMode);

   //private native int getNumFaces();
   //private native void getFacesInfo(int[] faceRect);

   /**
    * Function to pass camera frame for native, FastCV processing.
    * @param data Byte buffer for data.
    * @param w Width of data
    * @param h Height of data
    */
   //public native void updateFacedetect( byte[] data, int w, int h );


   /** Performs native cleanup routines for application exit. */
   //private native void cleanupFacedetect();

   /*************************************************************************/

   /**
    * Request render for on demand rendering.
    */
   public void requestRender()
   {
      if ( mGLSurface != null )
      {
         mGLSurface.requestRender();
      }
   }

   private void addDisplayPoint(int x, int y)
   {
      mPointTimeout[numActivePoints] = POINT_TIMEOUT;
      mPointX[numActivePoints] = x;
      mPointY[numActivePoints] = y;
      numActivePoints++;
   }

   private void deleteDisplayPoint(int index)
   {
      if(index < numActivePoints)
      {
         for(int i=index+1; i<numActivePoints; i++)
         {
            mPointTimeout[i-1] = mPointTimeout[i];
            mPointX[i-1] = mPointX[i];
            mPointY[i-1] = mPointY[i];
         }

         numActivePoints--;

         mPointTimeout[numActivePoints] = 0;
         mPointX[numActivePoints] = 0;
         mPointY[numActivePoints] = 0;
      }
   }
}
/*
 * 1) For preview resolution, the activity will take in a key 'resolution' in
 * string format, value would be 'width_height'. Default resolution used is
 * '800_480'.
 * If unsupported resolution is passed, then usua
 *
 * 2) For logging, one statement Conditional log added
 *
 * 3) For print screen, search for //for print screen.Pass in a key 'max_prints'
 * with an integer value above 0 to take prints, to not take prints, pass 0.
 * Currently, print rate is 3 per sec.Image name is
 * counter_prevwidth_prevheight_xbox_ybox_widthBox_heightBox.jpg
 * Subfolders are created within a root folder.
 * If subfolder already exists,  new subfolder with timestamp as its name is created.
 * No files or folders are deleted at any time.
 * For print screen feature, pass counter greater than 0.
 *
 * 4) //for text display on screen:
 *  changes in this file as well as in FastCVSampleView
 *
 * 5) //commandline dsp enable/disable:
 * pass in 'DSPEnable' with a value from 0 to any. 0 indicates no DSP, any non zero number indicates dsp mode.
 *
 */
