/*
 * rk_aiq_algo_agamma_itf.c
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

#include "agamma/rk_aiq_algo_agamma_itf.h"
#if RKAIQ_HAVE_GAMMA_V10
#include "agamma/rk_aiq_agamma_algo_v10.h"
#endif
#if RKAIQ_HAVE_GAMMA_V11
#include "agamma/rk_aiq_agamma_algo_v11.h"
#endif
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    AgammaHandle_t AgammaHandle;
} RkAiqAlgoContext;



static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t* pAgammaHandle = NULL;

    ret = AgammaInit(&pAgammaHandle, (CamCalibDbV2Context_t*)(cfg->calibv2));

    *context = (RkAiqAlgoContext *)(pAgammaHandle);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    AgammaHandle_t* pAgammaHandle = (AgammaHandle_t*)context;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AgammaRelease(pAgammaHandle);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t * pAgammaHandle = (AgammaHandle_t *)params->ctx;
    RkAiqAlgoConfigAgamma* pCfgParam = (RkAiqAlgoConfigAgamma*)params;
    rk_aiq_gamma_cfg_t *agamma_config = &pAgammaHandle->agamma_config;
    pAgammaHandle->working_mode = pCfgParam->com.u.prepare.working_mode;
    pAgammaHandle->prepare_type = pCfgParam->com.u.prepare.conf_type;

    if(!!(pAgammaHandle->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGI_AGAMMA("%s: Agamma Reload Para!!!\n", __FUNCTION__);
#if RKAIQ_HAVE_GAMMA_V10
        CalibDbV2_gamma_V10_t* calibv2_agamma_calib =
            (CalibDbV2_gamma_V10_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(pCfgParam->com.u.prepare.calibv2), agamma_calib));
        memcpy(&pAgammaHandle->agammaAttrV10.stAuto, calibv2_agamma_calib,
               sizeof(CalibDbV2_gamma_V10_t));  // reload iq
#endif
#if RKAIQ_HAVE_GAMMA_V11
        CalibDbV2_gamma_V11_t* calibv2_agamma_calib =
            (CalibDbV2_gamma_V11_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(pCfgParam->com.u.prepare.calibv2), agamma_calib));
        memcpy(&pAgammaHandle->agammaAttrV11.stAuto, calibv2_agamma_calib,
               sizeof(CalibDbV2_gamma_V11_t));  // reload iq
#endif
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t* pAgammaHandle = (AgammaHandle_t *)inparams->ctx;
    RkAiqAlgoProcResAgamma* pAgammaProcRes = (RkAiqAlgoProcResAgamma*)outparams;
    AgammaProcRes_t* pProcRes = (AgammaProcRes_t*)&pAgammaProcRes->GammaProcRes;

    AgammaProcessing(pAgammaHandle);

    //set proc res
    AgammaSetProcRes(pProcRes, &pAgammaHandle->agamma_config);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAgamma = {
    .common = {
        .version = RKISP_ALGO_AGAMMA_VERSION,
        .vendor  = RKISP_ALGO_AGAMMA_VENDOR,
        .description = RKISP_ALGO_AGAMMA_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AGAMMA,
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
