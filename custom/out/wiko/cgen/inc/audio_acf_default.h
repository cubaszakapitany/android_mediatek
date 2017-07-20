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
 * 04 22 2013 kh.hung
 * [ALPS00580843] [MT6572tdv1_phone[lca]][music][Symbio][Free test] 音?播放器播放音?的?音?大?小，不?定
 * Use default DRC.
 * 
 * Review: http://mtksap20:8080/go?page=NewReview&reviewid=59367
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
0x3e13a89e,   0x37815761,   0xca6a0000,     \
0x3decae7d,   0x36d05182,   0xcb420000,     \
0x3d38ccb9,   0x33b83346,   0xcf0e0000,     \
0x3c6bf4a5,   0x302f0b5a,   0xd3640000,     \
0x3c2602ab,   0x2efcfd54,   0xd4dd0000,     \
0x3af53b2c,   0x29b6c4d3,   0xdb540000,     \
    \
0x3f65a7cc,   0x3d565833,   0xc3440000,     \
0x3f58ae51,   0x3d1b51ae,   0xc38c0000,     \
0x3f1bd0de,   0x3c0d2f21,   0xc4d60000,     \
0x3ed20000,   0x3acd0000,   0xc65f0000,     \
0x3eb910c4,   0x3a5def3b,   0xc6e80000,     \
0x3e4653e7,   0x3862ac18,   0xc9560000,     \
    \
0x3ef6a1e6,   0x3b6c5e19,   0xc59c0000,     \
0x3ee0a738,   0x3b0958c7,   0xc6150000,     \
0x3e7ac371,   0x39473c8e,   0xc83e0000,     \
0x3e03eaa3,   0x3737155c,   0xcac50000,     \
0x3ddaf8f5,   0x3681070a,   0xcba40000,     \
0x3d2136f1,   0x3351c90e,   0xcf8d0000,     \
    \
0x3f9ba2b5,   0x3dec5d4a,   0xc2770000,     \
0x3f93a87e,   0x3dbe5781,   0xc2ad0000,     \
0x3f6bc78d,   0x3cea3872,   0xc3a90000,     \
0x3f3bf320,   0x3bed0cdf,   0xc4d60000,     \
0x3f2b030f,   0x3b95fcf0,   0xc53f0000,     \
0x3ede470b,   0x3a00b8f4,   0xc7200000,     \
    \
0x40c083a9,   0x3ca87c56,   0xc2960000,     \
0x40d18415,   0x3c5d7bea,   0xc2d00000,     \
0x411e863b,   0x3b0879c4,   0xc3d80000,     \
0x417a8945,   0x397076ba,   0xc5140000,     \
0x419a8a73,   0x38e1758c,   0xc5830000,     \
0x422d90a9,   0x36556f56,   0xc77c0000,     \
    \
0x405b830f,   0x3de37cf0,   0xc1c00000,     \
0x40638373,   0x3db47c8c,   0xc1e70000,     \
0x40888586,   0x3cda7a79,   0xc29c0000,     \
0x40b58896,   0x3bd47769,   0xc3750000,     \
0x40c589cf,   0x3b797630,   0xc3c10000,     \
0x410c9068,   0x39d36f97,   0xc51f0000,     \
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
0x281e503c,   0x281eb902,   0xe6840000,     \
0x2f065e0d,   0x2f06a687,   0xdd5d0000,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0
#define BES_LOUDNESS_WS_GAIN_MAX  0x0
#define BES_LOUDNESS_WS_GAIN_MIN  0x0
#define BES_LOUDNESS_FILTER_FIRST  0x0
#define BES_LOUDNESS_ATT_TIME  0x10e50054
#define BES_LOUDNESS_REL_TIME  0x263
#define BES_LOUDNESS_GAIN_MAP_IN \
0x0, 0x0, 0x0, 0x0, 0x0
#define BES_LOUDNESS_GAIN_MAP_OUT \
0x0, 0x0, 0x0, 0x0, 0x0
#endif
