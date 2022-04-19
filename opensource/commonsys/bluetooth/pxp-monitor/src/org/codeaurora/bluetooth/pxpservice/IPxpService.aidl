/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *        * Redistributions of source code must retain the above copyright
 *            notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright
 *            notice, this list of conditions and the following disclaimer in the
 *            documentation and/or other materials provided with the distribution.
 *        * Neither the name of The Linux Foundation nor
 *            the names of its contributors may be used to endorse or promote
 *            products derived from this software without specific prior written
 *            permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.    IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.codeaurora.bluetooth.pxpservice;

import android.bluetooth.BluetoothDevice;

import java.util.List;

interface IPxpService
{
    /**
     * Connect to the GATT service hosted on proximity reporter. The result of
     * connect operation is reported asynchronously through the Bluetooth GATT
     * callback.
     *
     * @param leDevice Bluetooth device
     * @return Return true if the connection where initiated successfully.
     */
    boolean connect(in BluetoothDevice leDevice);

    /**
     * Disconnect an existing connection with proximity reporter. The result of
     * disconnect operation is reported asynchronously through the Bluetooth
     * GATT callback.
     *
     * @param leDevice Bluetooth device
     * @return void
     */
    void disconnect(in BluetoothDevice leDevice);

    /**
     * Write link loss alert level to the remote device.
     *
     * @param leDevice Bluetooth remote device
     * @param level Alert level (0, 1 or 2).
     * @return Return true if the write operation was successful.
     */
    boolean setLinkLossAlertLevel(in BluetoothDevice leDevice, in int level);

    /**
     * Read link loss alert level from the remote device.
     *
     * @param leDevice Bluetooth remote device
     * @return Return value of link loss alert Level: 0 - off alert level 1 -
     *         mild alert level 2 - high alert level
     */
    int getLinkLossAlertLevel(in BluetoothDevice leDevice);

    /**
     * Set max value of path loss threshold.
     *
     * @param leDevice Bluetooth remote device
     * @return void
     */
    void setMaxPathLossThreshold(in BluetoothDevice leDevice, in int threshold);

    /**
     * Get max path loss threshold value.
     *
     * @param leDevice Bluetooth remote device
     * @return Return threshold value.
     */
    int getMaxPathLossThreshold(in BluetoothDevice leDevice);

    /**
     * Set max value of path loss threshold.
     *
     * @param leDevice Bluetooth remote device
     * @return void
     */
    void setMinPathLossThreshold(in BluetoothDevice leDevice, in int threshold);

    /**
     * Get min path loss threshold value.
     *
     * @param leDevice Bluetooth remote device
     * @return Return threshold value.
     */
    int getMinPathLossThreshold(in BluetoothDevice leDevice);

    /**
     * Write path loss alert level to the remote device.
     *
     * @param leDevice Bluetooth remote device
     * @param level Alert level (0, 1 or 2).
     * @return Return true if the write operation where initiated successfully.
     */
    boolean setPathLossAlertLevel(in BluetoothDevice leDevice, in int level);

    /**
     * Read path loss alert level from the remote device.
     *
     * @param leDevice Bluetooth remote device
     * @return Return value path loss alert Level: 0 - off alert level 1 - mild
     *         alert level 2 - high alert level
     */
    int getPathLossAlertLevel(in BluetoothDevice leDevice);

    /**
     * Get current connection state for a given device.
     *
     * @param leDevice Bluetooth remote device
     * @return true if device is connected
     */
    boolean getConnectionState(in BluetoothDevice leDevice);

    /**
     * Check if variable deviceProp for current device is set.
     *
     * @param leDevice Bluetooth remote device
     * @return true if is set
     */
    boolean isPropertiesSet(in BluetoothDevice leDevice);

    /**
     * Check if remote service was found on a given device
     *
     * @param leDevice Bluetooth remote device
     * @param service remote service
     * @return true if remote service was found
     */
    boolean checkServiceStatus(in BluetoothDevice leDevice, in int service);

    /**
     * Check status of reading Tx Power level characteristic operation.
     *
     * @param leDevice Bluetooth remote device
     * @return true if reading operation failed.
     */
    boolean checkFailedReadTxPowerLevel(in BluetoothDevice leDevice);

    /**
     * Disconnect all connected devices.
     */
    void disconnectDevices();

    /**
     * List connected devices to the proximity monitor.
     *
     * @return Return List<BluetoothDevice> of the connected devices.
     */
    List<BluetoothDevice> getConnectedDevices();
}

