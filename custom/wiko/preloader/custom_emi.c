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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   custom_emi.c
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This Module defines the EMI (external memory interface) related setting.
 *
 * Author:
 * -------
 *  EMI auto generator V0.01
 *
 *   Memory Device database last modified on 2014/6/5
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 *
 *------------------------------------------------------------------------------
 * WARNING!!!  WARNING!!!   WARNING!!!  WARNING!!!  WARNING!!!  WARNING!!! 
 * This file is generated by EMI Auto-gen Tool.
 * Please do not modify the content directly!
 * It could be overwritten!
 *============================================================================
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include "mt_emi.h"
             
#define NUM_EMI_RECORD 2
             
int num_of_emi_records = NUM_EMI_RECORD ;
             
EMI_SETTINGS emi_settings[] =
{            
          {
        	   0x202,  /* TYPE  eMMC + LPDDR2 */
        	   {0x90,0x01,0x4a,0x20,0x58,0x49,0x4e,0x59,0x48},  /* NAND_EMMC_ID */
        	   9,  /* EMMC and NAND ID/FW ID checking length */
        	   266,   /*EMI freq */
        	   0xAAAAAAAA,  /* EMI_DRVA_value */
        	   0x00AA0000,  /* EMI_DRVB_value */
        	   0x00000000,  /* EMI_ODLA_value */
        	   0x00000000,  /* EMI_ODLB_value */
        	   0x00000000,  /* EMI_ODLC_value */
        	   0x00000000,  /* EMI_ODLD_value */
        	   0x00000000,  /* EMI_ODLE_value */
        	   0x00000000,  /* EMI_ODLF_value */
        	   0x00000000,  /* EMI_ODLG_value */
        	   0x00000000,  /* EMI_ODLH_value */
        	   0x00000000,  /* EMI_ODLI_value */
        	   0x00000000,  /* EMI_ODLJ_value */
        	   0x00000000,  /* EMI_ODLK_value */
        	   0x00000000,  /* EMI_ODLL_value */
        	   0x00000000,  /* EMI_ODLM_value */
        	   0x00000000,  /* EMI_ODLN_value */
        	   
        	   0x02030000,  /* EMI_CONI_value */  /* set DRAM driving */
        	   0x40503763,  /* EMI_CONJ_value */
        	   0x23051020,  /* EMI_CONK_value */
        	   0x60428098,  /* EMI_CONL_value */
        	   0x00450800,  /* EMI_CONN_value */  /* remember set bank number */
        	   
        	   0x00000000,  /* EMI_DUTA_value */
        	   0x00000000,  /* EMI_DUTB_value */
        	   0x00000000,  /* EMI_DUTC_value */
        	   
        	   0x00000000,  /* EMI_DUCA_value*/
        	   0x00000000,  /* EMI_DUCB_value*/
        	   0x00000000,  /* EMI_DUCE_value*/
        	   
        	   0x00030000,  /* EMI_IOCL_value */
        	   
        	   0x00010000,  /* EMI_GEND_value */
        	   
        	   {0x20000000, 0x00000000,0,0},  /* DRAM RANK SIZE  4Gb/rank */
             {0,0}  /* reserved 2 */
        },
        {
        	   0x202,  /* TYPE  eMMC + LPDDR2 */
        	   { 0x90,0x01,0x4a,0x48,0x34,0x47,0x31,0x64,0x04},  /* NAND_EMMC_ID */
        	   9,  /* EMMC and NAND ID/FW ID checking length */
        	   266,   /*EMI freq */
        	   0xAAAAAAAA,  /* EMI_DRVA_value */
        	   0x00AA0000,  /* EMI_DRVB_value */
        	   0x00000000,  /* EMI_ODLA_value */
        	   0x00000000,  /* EMI_ODLB_value */
        	   0x00000000,  /* EMI_ODLC_value */
        	   0x00000000,  /* EMI_ODLD_value */
        	   0x00000000,  /* EMI_ODLE_value */
        	   0x00000000,  /* EMI_ODLF_value */
        	   0x00000000,  /* EMI_ODLG_value */
        	   0x00000000,  /* EMI_ODLH_value */
        	   0x00000000,  /* EMI_ODLI_value */
        	   0x00000000,  /* EMI_ODLJ_value */
        	   0x00000000,  /* EMI_ODLK_value */
        	   0x00000000,  /* EMI_ODLL_value */
        	   0x00000000,  /* EMI_ODLM_value */
        	   0x00000000,  /* EMI_ODLN_value */
        	   
        	   0x02030000,  /* EMI_CONI_value */  /* set DRAM driving */
        	   0x40503763,  /* EMI_CONJ_value */
        	   0x23051020,  /* EMI_CONK_value */
        	   0x60428098,  /* EMI_CONL_value */
        	   0x00450800,  /* EMI_CONN_value */  /* remember set bank number */
        	   
        	   0x00000000,  /* EMI_DUTA_value */
        	   0x00000000,  /* EMI_DUTB_value */
        	   0x00000000,  /* EMI_DUTC_value */
        	   
        	   0x00000000,  /* EMI_DUCA_value*/
        	   0x00000000,  /* EMI_DUCB_value*/
        	   0x00000000,  /* EMI_DUCE_value*/
        	   
        	   0x00030000,  /* EMI_IOCL_value */
        	   
        	   0x00010000,  /* EMI_GEND_value */
        	   
        	   {0x20000000, 0x00000000,0,0},  /* DRAM RANK SIZE  4Gb/rank */
             {0,0}  /* reserved 2 */
        }

}; 
