/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
 *
 * Filename:
 * ---------
 * audio_acf_default.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is the header of audio customization related parameters or definition.
 *
 * Author:
 * -------
 * Tina Tsai
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef AUDIO_ACF_DEFAULT_H
#define AUDIO_ACF_DEFAULT_H

   /* Compensation Filter HSF coeffs: default all pass filter       */
    /* BesLoudness also uses this coeffs    */ 
#define BES_LOUDNESS_HSF_COEFF \
0x7c7e9cb,   0xf0702c69,   0x7c7e9cb,   0x7c72c375,   0x0,     \
0x7c30718,   0xf079f1d0,   0x7c30718,   0x7c21c3c1,   0x0,     \
0x7ac72b3,   0xf0a71a9a,   0x7ac72b3,   0x7aabc51d,   0x0,     \
0x7915c50,   0xf0dd4760,   0x7915c50,   0x78e5c6ba,   0x0,     \
0x787de35,   0xf0f04396,   0x787de35,   0x7845c749,   0x0,     \
0x75c4ba5,   0xf14768b6,   0x75c4ba5,   0x755bc9d2,   0x0,     \
0x728aba0,   0xf1aea8bf,   0x728aba0,   0x71d5ccbf,   0x0,     \
0x716be89,   0xf1d282ed,   0x716be89,   0x7096cdbe,   0x0,     \
0x6c58c6d,   0xf274e725,   0x6c58c6d,   0x6ad4d222,   0x0,     \
    \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,   0x0,   0x0

#define BES_LOUDNESS_BPF_COEFF \
0x403c9023,   0x3e996fdc,   0xc1290000,     \
0x404192ed,   0x3e7a6d12,   0xc1430000,     \
0x405aa272,   0x3de95d8d,   0xc1bc0000,     \
0x4077ba2b,   0x3d3945d4,   0xc24e0000,     \
0x4082c397,   0x3cfc3c68,   0xc2810000,     \
0x40b2f3ca,   0x3bdf0c35,   0xc36d0000,     \
    \
0x3e7dbc6d,   0x3b224392,   0xc65f0000,     \
0x3e5dc5b5,   0x3ab93a4a,   0xc6e80000,     \
0x3dcaf45e,   0x38df0ba1,   0xc9560000,     \
0x3d1e2c53,   0x36b5d3ac,   0xcc2c0000,     \
0x3ce33d9f,   0x35f6c260,   0xcd250000,     \
0x0,   0x0,   0x0,     \
    \
0x3f95b55e,   0x3d254aa1,   0xc3440000,     \
0x3f8cbdf9,   0x3ce74206,   0xc38c0000,     \
0x3f62ea59,   0x3bc615a6,   0xc4d60000,     \
0x3f31228b,   0x3a6fdd74,   0xc65f0000,     \
0x3f1f34ed,   0x39f7cb12,   0xc6e80000,     \
0x0,   0x0,   0x0,     \
    \
0x403c9285,   0x3e2e6d7a,   0xc1950000,     \
0x404195ad,   0x3e066a52,   0xc1b80000,     \
0x4059a728,   0x3d4a58d7,   0xc25c0000,     \
0x4076c190,   0x3c683e6f,   0xc3200000,     \
0x4080cbf1,   0x3c19340e,   0xc3650000,     \
0x40af0000,   0x3aac0000,   0xc4a30000,     \
    \
0x3f74acec,   0x3aee5313,   0xc59c0000,     \
0x3f69b3fc,   0x3a814c03,   0xc6150000,     \
0x3f33d8c3,   0x388e273c,   0xc83e0000,     \
0x3ef50932,   0x3645f6cd,   0xcac50000,     \
0x3edf19e4,   0x357be61b,   0xcba40000,     \
0x3e7e584e,   0x31f3a7b1,   0xcf8d0000,     \
    \
0x40838a78,   0x3adb7587,   0xc4a10000,     \
0x408e8be9,   0x3a6b7416,   0xc5060000,     \
0x40c1939b,   0x386a6c64,   0xc6d40000,     \
0x40fd9ef7,   0x360d6108,   0xc8f50000,     \
0x4112a373,   0x353b5c8c,   0xc9b10000,     \
0x4170baf7,   0x31894508,   0xcd060000,     \
    \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
    \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0

#define BES_LOUDNESS_LPF_COEFF \
0x1bc91bc9,   0x86c,   0x0,     \
0x1da81da8,   0x4ae,   0x0,     \
0x265d265d,   0xf345,   0x0,     \
0x32793279,   0xdb0c,   0x0,     \
0x37cb37cb,   0xd069,   0x0,     \
0x0,   0x0,   0x0
#define BES_LOUDNESS_WS_GAIN_MAX  0x0
#define BES_LOUDNESS_WS_GAIN_MIN  0x0
#define BES_LOUDNESS_FILTER_FIRST  0x0
#define BES_LOUDNESS_ATT_TIME  0x0
#define BES_LOUDNESS_REL_TIME  0x0
#define BES_LOUDNESS_GAIN_MAP_IN \
0xd3, 0xd7, 0xd8, 0xee, 0x0
#define BES_LOUDNESS_GAIN_MAP_OUT \
0xc, 0xc, 0xc, 0xc, 0x0
#endif
