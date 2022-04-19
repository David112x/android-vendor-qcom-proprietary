/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
*/

#ifdef __cplusplus
extern "C"
{
#endif

bool hidl_client_initialize(int mode, int *tools_fd);
void hidl_client_close();

#ifdef __cplusplus
}
#endif

#define MODE_BT (0)
#define MODE_ANT (1)
#define MODE_FM (2)

