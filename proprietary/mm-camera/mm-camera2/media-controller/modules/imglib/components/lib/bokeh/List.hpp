/***************************************************************************
* Copyright (c) 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#ifndef __QCAMERA_LIST_H__
#define __QCAMERA_LIST_H__

template<class T>
class List {
    T listOfItems[32];
    int numItems;

    public:
        List()
        {
            numItems = 0;
        }
        void Add(T item)
        {
            listOfItems[numItems] = item;
            numItems++;
        }
        void Clear()
        {
            numItems = 0;
        }

        T Get(int index)
        {
            return listOfItems[index];
        }

        T Last()
        {
            return Get(numItems-1);
        }

        int Size()
        {
            return numItems;
        }
};

#endif
