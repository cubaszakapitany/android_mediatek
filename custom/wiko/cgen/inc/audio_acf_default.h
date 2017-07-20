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
#define BES_LOUDNESS_HSF_COEFF \
0x7abd4bb,   0xf0a85689,   0x7abd4bb,   0x7aacc532,   0x0,     \
0x7a49f3d,   0xf0b6c186,   0x7a49f3d,   0x7a36c5a2,   0x0,     \
0x7838d5e,   0xf0f8e543,   0x7838d5e,   0x7814c7a2,   0x0,     \
0x75c6895,   0xf1472ed6,   0x75c6895,   0x7587c9fa,   0x0,     \
0x74ed313,   0xf16259da,   0x74ed313,   0x74a2cac8,   0x0,     \
0x71156aa,   0xf1dd52ac,   0x71156aa,   0x708cce61,   0x0,     \
0x6ca45c1,   0xf26b747e,   0x6ca45c1,   0x6bbad272,   0x0,     \
0x6b2086c,   0xf29bef28,   0x6b2086c,   0x6a0fd3ce,   0x0,     \
0x646f238,   0xf3721b90,   0x646f238,   0x6289d9ab,   0x0,     \
    \
0x7dbab1d,   0xf048a9c5,   0x7dbab1d,   0x7da9c234,   0x0,     \
0x7d86aaa,   0xf04f2aab,   0x7d86aaa,   0x7d72c265,   0x0,     \
0x7c934f4,   0xf06d9618,   0x7c934f4,   0x7c6dc347,   0x0,     \
0x7b69179,   0xf092dd0e,   0x7b69179,   0x7b26c454,   0x0,     \
0x7afef79,   0xf0a0210e,   0x7afef79,   0x7ab0c4b2,   0x0,     \
0x790d1d9,   0xf0de5c4d,   0x790d1d9,   0x787ac660,   0x0,     \
0x76a8a5f,   0xf12aeb41,   0x76a8a5f,   0x75a9c858,   0x0,     \
0x75ce4e2,   0xf146363b,   0x75ce4e2,   0x74a2c905,   0x0,     \
0x71cd7a9,   0xf1c650ad,   0x71cd7a9,   0x6fa7cc0c,   0x0

#define BES_LOUDNESS_BPF_COEFF \
0x3f8fa0b2,   0x38905f4d,   0xc7df0000,     \
0x3f86a594,   0x37f25a6b,   0xc8860000,     \
0x3f5cbf43,   0x352940bc,   0xcb790000,     \
0x3f2ce2b9,   0x31ef1d46,   0xcee40000,     \
0x3f1befb5,   0x30d5104a,   0xd00e0000,     \
0x3ed128db,   0x2bf1d724,   0xd53c0000,     \
    \
0x3fa4a9ff,   0x39fb5600,   0xc65f0000,     \
0x3f9db086,   0x397a4f79,   0xc6e80000,     \
0x3f7ad297,   0x372e2d68,   0xc9560000,     \
0x3f520000,   0x34810000,   0xcc2c0000,     \
0x3f440fe7,   0x3395f018,   0xcd250000,     \
0x3f064e22,   0x2f79b1dd,   0xd1800000,     \
    \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
    \
0x4000c3ef,   0x38203c10,   0xc7df0000,     \
0x4000ce16,   0x377931e9,   0xc8860000,     \
0x40000000,   0x34860000,   0xcb790000,     \
0x4000388d,   0x311bc772,   0xcee40000,     \
0x400048de,   0x2ff1b721,   0xd00e0000,     \
0x0,   0x0,   0x0,     \
    \
0x4000d4b7,   0x311b2b48,   0xcee40000,     \
0x4000e026,   0x2ff11fd9,   0xd00e0000,     \
0x400014d4,   0x2ac3eb2b,   0xd53c0000,     \
0x40004761,   0x24f3b89e,   0xdb0c0000,     \
0x400052f8,   0x2300ad07,   0xdcff0000,     \
0x0,   0x0,   0x0,     \
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
0x0,   0x0,   0x0,     \
\
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0

#define BES_LOUDNESS_LPF_COEFF \
0x105b20b6,   0x105b09d2,   0xf4c00000,     \
0x12ad255b,   0x12ad0044,   0xf5040000,     \
0x1f593eb2,   0x1f59d337,   0xef640000,     \
0x352d6a5a,   0x352d977d,   0xd3cd0000,     \
0x3fad7f5b,   0x3fad80a5,   0xc0a40000,     \
0x0,   0x0,   0x0
#define BES_LOUDNESS_WS_GAIN_MAX  0x0
#define BES_LOUDNESS_WS_GAIN_MIN  0x0
#define BES_LOUDNESS_FILTER_FIRST  0x0
#define BES_LOUDNESS_ATT_TIME  0xa4
#define BES_LOUDNESS_REL_TIME  0x4010
#define BES_LOUDNESS_GAIN_MAP_IN \
0xd3, 0xda, 0xed, 0xee, 0x0
#define BES_LOUDNESS_GAIN_MAP_OUT \
0xc, 0xc, 0xc, 0xc, 0x0
#endif
