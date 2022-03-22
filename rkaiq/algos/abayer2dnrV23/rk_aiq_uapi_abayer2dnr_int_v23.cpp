#include "abayer2dnrV23/rk_aiq_uapi_abayer2dnr_int_v23.h"
#include "abayer2dnrV23/rk_aiq_types_abayer2dnr_algo_prvt_v23.h"
#include "RkAiqCalibApi.h"
//#include "bayer2dnr_xml2json_v23.h"



#if 1

#define RAWNR_LUMA_TF_STRENGTH_MAX_PERCENT (100.0)
#define RAWNR_LUMA_SF_STRENGTH_MAX_PERCENT (100.0)


XCamReturn
rk_aiq_uapi_abayer2dnrV23_SetAttrib(RkAiqAlgoContext *ctx,
                                    rk_aiq_bayer2dnr_attrib_v23_t *attr,
                                    bool need_sync)
{

    Abayer2dnr_Context_V23_t* pCtx = (Abayer2dnr_Context_V23_t*)ctx;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == ABAYER2DNR_V23_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == ABAYER2DNR_V23_OP_MODE_MANUAL) {
        pCtx->stManual.st2DSelect = attr->stManual.st2DSelect;
    } else if(pCtx->eMode == ABAYER2DNR_V23_OP_MODE_REG_MANUAL) {
        pCtx->stManual.st2Dfix = attr->stManual.st2Dfix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_abayer2dnrV23_GetAttrib(const RkAiqAlgoContext *ctx,
                                    rk_aiq_bayer2dnr_attrib_v23_t *attr)
{

    Abayer2dnr_Context_V23_t* pCtx = (Abayer2dnr_Context_V23_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_abayer2dnrV23_SetStrength(const RkAiqAlgoContext *ctx,
                                      float fPercent)
{
    Abayer2dnr_Context_V23_t* pCtx = (Abayer2dnr_Context_V23_t*)ctx;

    float fStrength = 1.0f;
    float fMax = RAWNR_LUMA_SF_STRENGTH_MAX_PERCENT;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        if(fPercent >= 0.999999)
            fPercent = 0.999999;
        fStrength = 0.5 / (1.0 - fPercent);
    }

    pCtx->fRawnr_SF_Strength = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}




XCamReturn
rk_aiq_uapi_abayer2dnrV23_GetStrength(const RkAiqAlgoContext *ctx,
                                      float *pPercent)
{
    Abayer2dnr_Context_V23_t* pCtx = (Abayer2dnr_Context_V23_t*)ctx;

    float fStrength = 1.0f;
    float fMax = RAWNR_LUMA_SF_STRENGTH_MAX_PERCENT;


    fStrength = pCtx->fRawnr_SF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        float tmp = 1.0;
        tmp = 1 - 0.5 / fStrength;
        if(abs(tmp - 0.999999) < 0.000001) {
            tmp = 1.0;
        }
        *pPercent = tmp;
    }

    return XCAM_RETURN_NO_ERROR;
}



#endif
