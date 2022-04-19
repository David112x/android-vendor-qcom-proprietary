/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 package com.android.bluetooth;

 import android.util.Log;

 import java.lang.reflect.Method;
 import java.lang.reflect.Field;
 import java.lang.reflect.InvocationTargetException;
 import java.util.ArrayList;
 import java.util.HashMap;

/* This API is added to access data members or member functions from the class which
 * is defined in other component and the required memebers may not be available(or merged)
 * in other component. This API helps avoiding compilation issues. */
 public class ReflectionUtils {

    private static final String TAG = "BluetoothReflectionUtils";

    private static final HashMap<String, Class> primitiveClsMap =
        new HashMap<String, Class>();

    static {
        primitiveClsMap.put("java.lang.Integer", int.class);
        primitiveClsMap.put("java.lang.Long", long.class);
        primitiveClsMap.put("java.lang.Character", char.class);
        primitiveClsMap.put("java.lang.Byte", byte.class);
        primitiveClsMap.put("java.lang.Float", float.class);
        primitiveClsMap.put("java.lang.Double", double.class);
        primitiveClsMap.put("java.lang.Short", short.class);
        primitiveClsMap.put("java.lang.Boolean", boolean.class);
    }

    /**********************************************************************************
     * Function     :  getField
     *
     * Description  :  fetches mentioned data member from the provided object.
     *                 Access is restricted to public data members if package of the
     *                 required object class is different from ReflectionUtils package.
     *                 If package is same, protected and default can aslo be accessed.
     *                 obj       - Object in which data member could be present
     *                 fieldName - name of the data member in string format
     *
     * Returns      :  data member as 'Field'
     *                 null if data member is not found
     **********************************************************************************/
    public Field getField(Object obj, String fieldName) {
        Log.d(TAG, "getField {" + fieldName + "} class: " + obj.getClass().getName());
        Field field = null;
        try {
            // get object class
            Class cls = obj.getClass();
            // get required field. NoSuchFieldException is thrown if field is not found
            field = cls.getDeclaredField(fieldName);
        } catch(NoSuchFieldException e) {
            Log.e(TAG, "Field(" + fieldName + ") not found in class:" + obj.getClass().getName()
                        +". Exception : " + e);
        }
        return field;
    }

    /**********************************************************************************
     * Function     :  isMethodAvailable
     *
     * Description  :  checks if mentioned method is available in given object's class.
     *                 Access is restricted to public member functions only if package
     *                 of required object class is different from ReflectionUtils package.
     *                 If package is same, protected and default func can also be accessed.
     *                 obj        - Object in which data member could be present
     *                 methodName - name of the member function in string format
     *                 args       - all method arguments wrapped in Object format in
     *                              arraylist
     * Returns      :  true if method is present otherwise returns false
     **********************************************************************************/
    public boolean isMethodAvailable(Object obj, String methodName, ArrayList<Object> args) {
        Log.d(TAG, "isMethodAvailable: " + methodName + ", Class: " + obj.getClass().getName());
        boolean isAvailable = true;
        Class[] cArgs = null;
        try {
            Class cls = obj.getClass();
            // get parameter types of function call
            if (args != null && args.size() > 0) {
                int i =0;
                cArgs = new Class[args.size()];
                for (Object objs : args) {
                    if (isWrapperType(objs.getClass().getName())) {
                        cArgs[i++] = primitiveClsMap.get(objs.getClass().getName());
                    } else {
                        cArgs[i++] = objs.getClass();
                    }
                }
            }
            Method m = cls.getDeclaredMethod(methodName, cArgs);
        } catch (NoSuchMethodException e) {
            isAvailable = false;
            Log.e(TAG, "Method not found in class:" + obj.getClass().getName()
                    +". Exception :" + e);
        }
        return isAvailable;
    }

    /**********************************************************************************
     * Function     :  invokeMethod
     *
     * Description  :  This is a generic method which calls required method.
     *                 Access is restricted to public member functions only if package
     *                 of required object class is different from ReflectionUtils package.
     *                 If package is same, protected and default func can also be accessed.
     *                 obj        - Object in which data member could be present
     *                 methodName - name of the member function in string format
     *                 args       - all method arguments wrapped in Object format in
     *                              arraylist
     * Returns      :  return type of the required method in 'Object' format.
     *                 Typecasting should be done to required type.
     **********************************************************************************/
    public Object invokeMethod(Object obj, String methodName, ArrayList<Object> args) {
        Log.d(TAG, "invokeMethod: " + methodName + ", From: " + obj.getClass().getName());
        Object returnValue = null;
        Class[] cArgs = null;
        try {
            // Obtain object class to call required method
            Class cls = obj.getClass();

            // create function param signeture
            if (args != null && args.size() > 0) {
                int i =0;
                cArgs = new Class[args.size()];
                for (Object objs : args) {
                    if (isWrapperType(objs.getClass().getName())) {
                        cArgs[i++] = primitiveClsMap.get(objs.getClass().getName());
                    } else {
                        cArgs[i++] = objs.getClass();

                    }
                }
            }

            // Obtain required method.
            // throws exeption NoSuchMethodException if method is not found
            Method m = cls.getDeclaredMethod(methodName, cArgs);

            // Invoke required method call
            if (args != null)
                returnValue = m.invoke(obj, args.toArray());
            else
                returnValue = m.invoke(obj);
        } catch (NoSuchMethodException|IllegalAccessException|InvocationTargetException e) {
            Log.e(TAG, "Exception in invokeMethod: " + e);
        }
        return returnValue;
    }

    // Checks if a class belongs to Wrapper class of primitive data types
    public boolean isWrapperType(String classType) {
      return primitiveClsMap.containsKey(classType);
    }
  }

