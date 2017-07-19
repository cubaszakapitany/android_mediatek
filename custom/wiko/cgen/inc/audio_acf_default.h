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

    /* Compensation Filter BPF coeffs: default all pass filter      */ 
#define BES_LOUDNESS_BPF_COEFF \
0x400b8242,   0x3e647dbd,   0xc1900000,     \
0x400c8286,   0x3e407d79,   0xc1b20000,     \
0x401183e3,   0x3d997c1c,   0xc2540000,     \
0x401785d6,   0x3cd27a29,   0xc3160000,     \
0x4019869a,   0x3c8b7965,   0xc35a0000,     \
0x40228aa9,   0x3b497556,   0xc4930000,     \
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
0x0,   0x0,   0x0,     \
    \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0

#define BES_LOUDNESS_LPF_COEFF \
0x1ac33586,   0x1ac3e25f,   0xf2940000,     \
0x1ecb3d96,   0x1ecbd4fc,   0xefd50000,     \
0x37b36f66,   0x37b391ad,   0xcf840000,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0,     \
0x0,   0x0,   0x0
#define BES_LOUDNESS_WS_GAIN_MAX  0x0
#define BES_LOUDNESS_WS_GAIN_MIN  0x0
#define BES_LOUDNESS_FILTER_FIRST  0x0
#define BES_LOUDNESS_ATT_TIME  0x0
#define BES_LOUDNESS_REL_TIME  0x0
#define BES_LOUDNESS_GAIN_MAP_IN \
0xd5, 0xda, 0xe2, 0xee, 0x0
#define BES_LOUDNESS_GAIN_MAP_OUT \
0xf, 0xf, 0xf, 0xf, 0x0
#endif
