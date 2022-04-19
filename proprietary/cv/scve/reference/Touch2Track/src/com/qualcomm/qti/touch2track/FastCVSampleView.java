/*==============================================================================
 * Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/
package com.qualcomm.qti.touch2track;

import java.util.ArrayList;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Matrix;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class FastCVSampleView extends View
{
   private final String TAG = "FastCVSampleView";
   private ArrayList<Rect> mRects = new ArrayList<Rect>();
   private ArrayList<Integer> mRectColors = new ArrayList<Integer>();

   private ArrayList<Point> mPoints = new ArrayList<Point>();
   private ArrayList<Integer> mPointColors = new ArrayList<Integer>();


   private Paint mPaint;

   private int mLineWidth = 3;
   private int mPointRadius = 3;

   // for text display on screen
   private String mFPS = "";
   private String mResolution = "";

   public void setFPS(String aFPS){
      mFPS = aFPS;
   }

   public void setResolution(String res){
      mResolution = res;
   }
   //end


   public FastCVSampleView(Context context) {
      super(context);
      mPaint = new Paint();
   }

   @Override
   protected void onDraw(Canvas canvas)
   {
      int width = canvas.getWidth();
      int height = canvas.getHeight();

      //draw the rectangles
      mPaint.setStrokeWidth(mLineWidth);
      mPaint.setStyle(Paint.Style.STROKE);
      for (int i=0;i<mRects.size();i++)
      {
         if (mRects.get(i).left <= mLineWidth || mRects.get(i).right >= width-mLineWidth-1
             ||  mRects.get(i).top <= mLineWidth || mRects.get(i).bottom >= height-mLineWidth-1 )
            continue;
         mPaint.setColor(mRectColors.get(i));
         canvas.drawRect(mRects.get(i), mPaint);
      }

      //draw the points
      mPaint.setStrokeWidth(0);
      mPaint.setStyle(Paint.Style.FILL);
      for (int i=0;i<mPoints.size();i++)
      {
         if (mPoints.get(i).x <= mPointRadius || mPoints.get(i).x >= width-mPointRadius-1
             || mPoints.get(i).y <= mPointRadius || mPoints.get(i).y >= height-mPointRadius-1 )
            continue;
         mPaint.setColor(mPointColors.get(i));
         canvas.drawCircle(mPoints.get(i).x, mPoints.get(i).y, mPointRadius, mPaint);
      }

      //for text display on screen
      mPaint.setTextSize(30.0f);
      mPaint.setStrokeWidth(5);
      mPaint.setColor(Color.GREEN);
      canvas.drawText("FPS: " + mFPS, 0, 0.95f * getHeight(), mPaint);
      canvas.drawText("Resolution: " + mResolution, 0, 0.85f * getHeight(), mPaint);
      //end

      super.onDraw(canvas);
   }

   public void clean()
   {
      mRects.clear();
      mRectColors.clear();
      mPoints.clear();
      mPointColors.clear();
   }

   public void addRect(int x, int y, int w, int h, int color)
   {
      Rect r= new Rect(x, y, x+w-1, y+h-1);
      mRects.add(r);
      mRectColors.add(color);
   }

   public void addPoint(int x, int y, int color)
   {
      Point p = new Point(x,y);
      mPoints.add(p);
      mPointColors.add(color);
   }
}
