/*
 * rk_aiq_algo_camgroup_adhaz_itf.c
 *
 *  Copyright (c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
/* for rockchip v2.0.0*/

#include "rk_aiq_algo_types_int.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "algos/adehaze/rk_aiq_algo_adhaz_itf.h"
#include "RkAiqCalibDbTypes.h"
#include "algos/adehaze/rk_aiq_adehaze_algo.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t *AdehazeGrpHandle = NULL;
    AlgoCtxInstanceCfgCamGroup* instanc_int = (AlgoCtxInstanceCfgCamGroup*)cfg;

    ret = AdehazeInit(&AdehazeGrpHandle, (CamCalibDbV2Context_t*)(instanc_int->s_calibv2));

    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ADEHAZE("%s Adehaze Init failed: %d", __FUNCTION__, ret);
        return(XCAM_RETURN_ERROR_FAILED);
    }

    *context = (RkAiqAlgoContext *)(AdehazeGrpHandle);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t *AdehazeGrpHandle = (AdehazeHandle_t *)context;

    ret = AdehazeRelease(AdehazeGrpHandle);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoCamGroupPrepare* config = (RkAiqAlgoCamGroupPrepare*)params;
    AdehazeHandle_t * AdehazeGrpHandle = (AdehazeHandle_t *)params->ctx;
    const CamCalibDbV2Context_t* pCalibDb = config->s_calibv2;

    AdehazeGrpHandle->working_mode = config->gcom.com.u.prepare.working_mode;

    if (AdehazeGrpHandle->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        AdehazeGrpHandle->FrameNumber = LINEAR_NUM;
    else if (AdehazeGrpHandle->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AdehazeGrpHandle->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        AdehazeGrpHandle->FrameNumber = HDR_2X_NUM;
    else
        AdehazeGrpHandle->FrameNumber = HDR_3X_NUM;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGD_ADEHAZE("%s: Adehaze Reload Para!\n", __FUNCTION__);

        if(AdehazeGrpHandle->HWversion == ADEHAZE_ISP21) {
            CalibDbV2_dehaze_V20_t* calibv2_adehaze_calib_V20 =
                (CalibDbV2_dehaze_V20_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib_v20));
            if (calibv2_adehaze_calib_V20)
                memcpy(&AdehazeGrpHandle->Calib.Dehaze_v20, calibv2_adehaze_calib_V20, sizeof(CalibDbV2_dehaze_V20_t));
        }
        else if(AdehazeGrpHandle->HWversion == ADEHAZE_ISP21) {
            CalibDbV2_dehaze_V21_t* calibv2_adehaze_calib_V21 =
                (CalibDbV2_dehaze_V21_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib_v21));
            if (calibv2_adehaze_calib_V21)
                memcpy(&AdehazeGrpHandle->Calib.Dehaze_v21, calibv2_adehaze_calib_V21, sizeof(CalibDbV2_dehaze_V21_t));
        }
        else if(AdehazeGrpHandle->HWversion == ADEHAZE_ISP30) {
            CalibDbV2_dehaze_V30_t* calibv2_adehaze_calib_V30 =
                (CalibDbV2_dehaze_V30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adehaze_calib_v30));
            if (calibv2_adehaze_calib_V30)
                memcpy(&AdehazeGrpHandle->Calib.Dehaze_v30, calibv2_adehaze_calib_V30, sizeof(CalibDbV2_dehaze_V30_t));
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;

}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t * AdehazeGrpHandle = (AdehazeHandle_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* pGrpProcPara = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* pGrpProcResPara = (RkAiqAlgoCamGroupProcOut*)outparams;

    LOGD_ADEHAZE("/*************************Adehaze Group Start******************/ \n");

    AdehazeGetCurrDataGroup(AdehazeGrpHandle, &pGrpProcPara->camgroupParmasArray[0]->aec._effAecExpInfo,
                            pGrpProcPara->camgroupParmasArray[0]->aec._aePreRes);

    //process
    if(!(AdehazeByPassProcessing(AdehazeGrpHandle)))
        ret = AdehazeProcess(AdehazeGrpHandle, AdehazeGrpHandle->HWversion);

    //store data
    if(AdehazeGrpHandle->HWversion == ADEHAZE_ISP20)
        AdehazeGrpHandle->PreData.V20.ApiMode = AdehazeGrpHandle->AdehazeAtrr.AdehazeAtrrV20.mode;
    else if(AdehazeGrpHandle->HWversion == ADEHAZE_ISP21)
        AdehazeGrpHandle->PreData.V21.ApiMode = AdehazeGrpHandle->AdehazeAtrr.AdehazeAtrrV21.mode;
    else if(AdehazeGrpHandle->HWversion == ADEHAZE_ISP30)
        AdehazeGrpHandle->PreData.V30.ApiMode = AdehazeGrpHandle->AdehazeAtrr.AdehazeAtrrV30.mode;

    //proc res
    if(AdehazeGrpHandle->HWversion == ADEHAZE_ISP20) {
        AdehazeGrpHandle->ProcRes.ProcResV20.enable = true;
        AdehazeGrpHandle->ProcRes.ProcResV20.update = !(AdehazeGrpHandle->byPassProc) ;
    }
    else if(AdehazeGrpHandle->HWversion == ADEHAZE_ISP21) {
        AdehazeGrpHandle->ProcRes.ProcResV21.enable = true;
        AdehazeGrpHandle->ProcRes.ProcResV21.update = !(AdehazeGrpHandle->byPassProc);
    }
    else if(AdehazeGrpHandle->HWversion == ADEHAZE_ISP30) {
        AdehazeGrpHandle->ProcRes.ProcResV30.enable = true;
        AdehazeGrpHandle->ProcRes.ProcResV30.update = !(AdehazeGrpHandle->byPassProc);
    }

    for (int i = 0; i < pGrpProcResPara->arraySize; i++)
        memcpy(pGrpProcResPara->camgroupParmasArray[i]->_adehazeConfig, &AdehazeGrpHandle->ProcRes, sizeof(RkAiqAdehazeProcResult_t));

    LOGD_ADEHAZE("/*************************Adehaze Group Over******************/ \n");

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAdhaz = {
    .common = {
        .version = RKISP_ALGO_ADHAZ_VERSION,
        .vendor  = RKISP_ALGO_ADHAZ_VENDOR,
        .description = RKISP_ALGO_ADHAZ_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADHAZ,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
