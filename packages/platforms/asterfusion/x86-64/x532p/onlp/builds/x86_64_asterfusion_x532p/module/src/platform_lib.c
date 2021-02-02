/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2021 Asterfusion Data Technologies Co., Ltd.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <AIM/aim.h>

#include "platform_lib.h"


int pltfm_qsfp_present_get(int port, int *mask)
{
    int error = ONLP_STATUS_E_UNSUPPORTED;
    FILE *fp;
    char data[256] = {0};

    fp = fopen("/var/asterfusion/qsfp_presence", "r");
    if(!fp) {
      return ONLP_STATUS_E_INTERNAL;
    } else {
      if (fgets(data, 256 - 1, fp) != NULL) {
          *mask = strtol(data, NULL, 16);
          *mask ^= 0xFFFFFFFF;
          fclose(fp);
          error = ONLP_STATUS_OK;
      }
    }
    return error;
}

int 
pltfm_thermal_get(onlp_thermal_info_t* info, int thermal_id)
{
    int error = ONLP_STATUS_E_INTERNAL;
    FILE *fp;
    char f[32] = {0};
    char data[32] = {0};
    
    sprintf(f, "/var/asterfusion/thermal_%d_temp", thermal_id);
    fp = fopen(f, "r");
    if (!fp) {
        goto finish;
    } else {
        if(fgets (data, 32, fp) != NULL) {
            /* Thermals */
            info->mcelsius = atoi(data);
            error = ONLP_STATUS_OK;
        }
    }
finish:
    return error;
}


int 
pltfm_psu_get(onlp_psu_info_t* info, int id)
{
    int error = 0;
    FILE *fp;
    char f[32] = {0};
    char data[32] = {0};

    info->caps = 0;
    info->caps = ONLP_PSU_CAPS_AC | ONLP_PSU_CAPS_VIN | ONLP_PSU_CAPS_IOUT | ONLP_PSU_CAPS_VOUT;

    sprintf(f, "/var/asterfusion/psu_%d_vin", id);
    fp = fopen(f, "r");
    if (!fp) {
        goto finish;
    } else {
        if(fgets (data, 32, fp) != NULL) {
            info->mvin = atoi(data);
            error = ONLP_STATUS_OK;
        }
    }

    sprintf(f, "/var/asterfusion/psu_%d_vout", id);
    fp = fopen(f, "r");
    if (!fp) {
        goto finish;
    } else {
        if(fgets (data, 32, fp) != NULL) {
            info->mvout = atoi(data);
            error = ONLP_STATUS_OK;
        }
    }

    sprintf(f, "/var/asterfusion/psu_%d_iout", id);
    fp = fopen(f, "r");
    if (!fp) {
        goto finish;
    } else {
        if(fgets (data, 32, fp) != NULL) {
            info->miout = atoi(data);
            error = ONLP_STATUS_OK;
        }
    }

finish:
    return error;
}

int 
pltfm_fan_info_get(onlp_fan_info_t* info, int fan_id)
{
    int error = 0;
    FILE *fp;
    char f[32] = {0};
    char data[32] = {0};

    sprintf(f, "/var/asterfusion/fan_%d_rpm", fan_id);
    fp = fopen(f, "r");
    if (!fp) {
        goto finish;
    } else {
        if(fgets (data, 32, fp) != NULL) {
            info->rpm = atoi(data);
            error = ONLP_STATUS_OK;
        }
    }

    info->status |= ONLP_FAN_STATUS_PRESENT | ONLP_FAN_STATUS_F2B;

finish:
    return error;
}

int
pltfm_psu_present_get(int *exist, int id)
{
    int error = ONLP_STATUS_E_INTERNAL;
    FILE *fp;
    char f[32] = {0};
    int present = 0;
    
    sprintf(f, "/var/asterfusion/psu_%d_presence", id);
    fp = fopen(f, "r");
    if (!fp) {
        goto finish;
    } else {
        *exist = 0;
        present = fgetc(fp);
        if(feof(fp)) {
            goto finish;
        } else {
            if (present == '1') {
                *exist = 1;  /* present */
            } else {
                *exist = 0;  /* absent */
            }
            error = ONLP_STATUS_OK;
       }
    }
finish:
    return error;
}


int
pltfm_psu_pwgood_get(int *pw_good, int id)
{
    /* PSU Power */
    if (id == PSU1_ID) {
      *pw_good = 1;  /* PSU1 Power */
    }
    if (id == PSU2_ID) {
      *pw_good = 1;  /* PSU2 Power */
    }

    return ONLP_STATUS_OK;
}


int
pltfm_onie_info_get(onlp_onie_info_t* onie)
{
    int error = ONLP_STATUS_E_INTERNAL;
    FILE *fp;
    char f[128] = {0};
    char f1[64] = {0};
    char f2[64] = {0};
    
    sprintf(f, "/var/asterfusion/eeprom");
    fp = fopen(f, "r");
    if (!fp) {
        goto finish;
    } else {
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Product Name 0x21 %s", f1);
            onie->product_name = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Part Number 0x22 %s", f1);
            onie->part_number = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Serial Number 0x23 %s", f1);
            onie->serial_number = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            uint32_t mac[6];
            AIM_SSCANF(f, "Base MAC Addre 0x24 %x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
            onie->mac[0] = mac[0];
            onie->mac[1] = mac[1];
            onie->mac[2] = mac[2];
            onie->mac[3] = mac[3];
            onie->mac[4] = mac[4];
            onie->mac[5] = mac[5];
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Manufacture Date 0x25 %s %s", f1, f2);
            sprintf(f, "%s %s", f1, f2);
            onie->manufacture_date = aim_memdup(f, strlen(f));
        }
        if( fgets (f, 128, fp) != NULL ) {
            uint32_t device_version;
            AIM_SSCANF(f, "Device Version 0x26 %d", &device_version);
            onie->device_version = device_version;
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Label Revision 0x27 %s", f1);
            onie->label_revision = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Platform Name 0x28 %s", f1);
            onie->platform_name = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Onie Version 0x29 %s", f1);
            onie->onie_version = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            uint32_t mac_range;
            AIM_SSCANF(f, "MAC Addresses 0x2a %d", &mac_range);
            onie->mac_range = mac_range;
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Manufacturer 0x2b %s", f1);
            onie->manufacturer = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Country Code 0x2c %s", f1);
            onie->country_code = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Vendor Name 0x2d %s", f1);
            onie->vendor = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Diag Version 0x2e %s", f1);
            onie->diag_version = aim_memdup(f1, strlen(f1));
        }
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "Service Tag 0x2f %s", f1);
            onie->service_tag = aim_memdup(f1, strlen(f1));
        }
        fgets (f, 128, fp);
        fgets (f, 128, fp);
        fgets (f, 128, fp);
        fgets (f, 128, fp);
        fgets (f, 128, fp);
        if( fgets (f, 128, fp) != NULL ) {
            AIM_SSCANF(f, "CRC-32 0xfe 0x%8x", &onie->crc);
        }

        /* To avoid Segmentation fault*/
        list_init(&onie->vx_list);
        onie->_hdr_id_string = aim_memdup("X", strlen("X"));

        fclose(fp);
        error = ONLP_STATUS_OK;
    }

finish:
    return error;
}



