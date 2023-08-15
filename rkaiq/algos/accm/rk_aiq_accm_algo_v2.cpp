/*
* rk_aiq_accm_algo_v1.cpp

* for rockchip v2.0.0
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

#include "accm/rk_aiq_accm_algo_com.h"
#include "interpolation.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

void CCMV2PrintReg(const rk_aiq_ccm_cfg_v2_t* hw_param) {
    LOG1_ACCM(
        " CCM V2 reg values: "
        " sw_ccm_asym_adj_en %d"
        " sw_ccm_enh_adj_en %d"
        " sw_ccm_highy_adjust %d"
        " sw_ccm_en_i %d"
        " sw_ccm_coeff ([%f,%f,%f,%f,%f,%f,%f,%f,%f]-E)X128"
        " sw_ccm_offset [%f,%f,%f]"
        " sw_ccm_coeff_y [%f,%f,%f]"
        " sw_ccm_alp_y [%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]"
        " sw_ccm_right_bit %f"
        " sw_ccm_bound_bit %f"
        " sw_ccm_color_coef_y [%d,%d,%d]"
        " sw_ccm_color_enh_rat_max %f",
        hw_param->asym_adj_en, hw_param->enh_adj_en, hw_param->highy_adj_en, hw_param->ccmEnable,
        hw_param->matrix[0], hw_param->matrix[1], hw_param->matrix[2], hw_param->matrix[3],
        hw_param->matrix[4], hw_param->matrix[5], hw_param->matrix[6], hw_param->matrix[7],
        hw_param->matrix[8], hw_param->offs[0], hw_param->offs[1], hw_param->offs[2],
        hw_param->rgb2y_para[0], hw_param->rgb2y_para[1], hw_param->rgb2y_para[2],
        hw_param->alp_y[0], hw_param->alp_y[1], hw_param->alp_y[2], hw_param->alp_y[3],
        hw_param->alp_y[4], hw_param->alp_y[5], hw_param->alp_y[6], hw_param->alp_y[7],
        hw_param->alp_y[8], hw_param->alp_y[9], hw_param->alp_y[10], hw_param->alp_y[11],
        hw_param->alp_y[12], hw_param->alp_y[13], hw_param->alp_y[14], hw_param->alp_y[15],
        hw_param->alp_y[16], hw_param->alp_y[17], hw_param->right_bit, hw_param->bound_bit,
        hw_param->enh_rgb2y_para[0], hw_param->enh_rgb2y_para[1], hw_param->enh_rgb2y_para[2],
        hw_param->enh_rat_max);
}

void CCMV2PrintDBG(const accm_context_t* accm_context) {
    const CalibDbV2_Ccm_Para_V32_t* pCcm = accm_context->ccm_v2;
    const float* pMatrixUndamped         = accm_context->accmRest.undampedCcmMatrix;
    const float* pOffsetUndamped         = accm_context->accmRest.undampedCcOffset;
    const float* pMatrixDamped           = accm_context->ccmHwConf_v2.matrix;
    const float* pOffsetDamped           = accm_context->ccmHwConf_v2.offs;

    LOG1_ACCM(
        "Illu Probability Estimation Enable: %d"
        "color_inhibition sensorGain: %f,%f,%f,%f "
        "color_inhibition level: %f,%f,%f,%f"
        "color_saturation sensorGain: %f,%f,%f,%f "
        "color_saturation level: %f,%f,%f,%f"
        "dampfactor: %f"
        " undampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f"
        " undampedCcOffset: %f,%f,%f "
        " dampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f"
        " dampedCcOffset:%f,%f,%f",
        pCcm->TuningPara.illu_estim.interp_enable,
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[0],
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[1],
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[2],
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[3],
        accm_context->mCurAttV2.stAuto.color_inhibition.level[0],
        accm_context->mCurAttV2.stAuto.color_inhibition.level[1],
        accm_context->mCurAttV2.stAuto.color_inhibition.level[2],
        accm_context->mCurAttV2.stAuto.color_inhibition.level[3],
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[0],
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[1],
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[2],
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[3],
        accm_context->mCurAttV2.stAuto.color_saturation.level[0],
        accm_context->mCurAttV2.stAuto.color_saturation.level[1],
        accm_context->mCurAttV2.stAuto.color_saturation.level[2],
        accm_context->mCurAttV2.stAuto.color_saturation.level[3], pCcm->TuningPara.damp_enable,
        pMatrixUndamped[0], pMatrixUndamped[1], pMatrixUndamped[2], pMatrixUndamped[3],
        pMatrixUndamped[4], pMatrixUndamped[5], pMatrixUndamped[6], pMatrixUndamped[7],
        pMatrixUndamped[8], pOffsetUndamped[0], pOffsetUndamped[1], pOffsetUndamped[2],
        pMatrixDamped[0], pMatrixDamped[1], pMatrixDamped[2], pMatrixDamped[3], pMatrixDamped[4],
        pMatrixDamped[5], pMatrixDamped[6], pMatrixDamped[7], pMatrixDamped[8], pOffsetDamped[0],
        pOffsetDamped[1], pOffsetDamped[2]);
}

XCamReturn AccmAutoConfig(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (hAccm == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    const CalibDbV2_Ccm_Para_V32_t* pCcm = NULL;
    float sensorGain                     = hAccm->accmSwInfo.sensorGain;
    float fSaturation                    = 0;
    pCcm = hAccm->ccm_v2;
    if (hAccm->update) {
        if (pCcm->TuningPara.illu_estim.interp_enable) {
            hAccm->isReCal_ = true;
            ret = interpCCMbywbgain(&pCcm->TuningPara, hAccm, fSaturation);
            RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
        } else {
            ret = selectCCM(&pCcm->TuningPara, hAccm, fSaturation);
            RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
        }
    }
    // 4) calc scale for y_alpha_curve and
    if (hAccm->update || hAccm->updateAtt) {
        float fScale = 1.0;
#if 1
        // real use
        interpolation(pCcm->lumaCCM.gain_alphaScale_curve.gain,
                      pCcm->lumaCCM.gain_alphaScale_curve.scale, 9, sensorGain, &fScale);
#else
        // for test, to be same with demo
        for (int i = 0; i < 9; i++) {
            int j = uint16_t(sensorGain);
            j     = (j > (1 << 8)) ? (1 << 8) : j;

            if (j <= (1 << i)) {
                fScale = pCcm->lumaCCM.gain_alphaScale_curve.scale[i];
                break;
            }
        }
#endif
        // 5) color inhibition adjust for api
        interpolation(hAccm->mCurAttV2.stAuto.color_inhibition.sensorGain,
                      hAccm->mCurAttV2.stAuto.color_inhibition.level, RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &hAccm->accmRest.color_inhibition_level);

        if (hAccm->accmRest.color_inhibition_level > 100 || hAccm->accmRest.color_inhibition_level < 0) {
            LOGE_ACCM("flevel2: %f is out of range [0 100]\n", hAccm->accmRest.color_inhibition_level);
            return XCAM_RETURN_ERROR_PARAM;
        }

        fScale *= (100 - hAccm->accmRest.color_inhibition_level) / 100;

        // 6)   saturation adjust for api
        float saturation_level = 100;
        interpolation(hAccm->mCurAttV2.stAuto.color_saturation.sensorGain,
                      hAccm->mCurAttV2.stAuto.color_saturation.level, RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &saturation_level);

        if (saturation_level > 100 || saturation_level < 0) {
            LOGE_ACCM("flevel1: %f is out of range [0 100]\n", saturation_level);
            return XCAM_RETURN_ERROR_PARAM;
        }

        LOGD_ACCM("CcmProfile changed: %d, fScale: %f->%f, sat_level: %f->%f",
            hAccm->isReCal_, hAccm->accmRest.fScale, fScale,
            hAccm->accmRest.color_saturation_level, saturation_level);

        hAccm->isReCal_ = hAccm->isReCal_ ||
                        fabs(fScale - hAccm->accmRest.fScale) > DIVMIN ||
                        fabs(saturation_level - hAccm->accmRest.color_saturation_level) > DIVMIN;

        if (hAccm->isReCal_) {
            hAccm->accmRest.fScale = fScale;
            hAccm->accmRest.color_saturation_level = saturation_level;
            Saturationadjust(fScale, saturation_level, hAccm->accmRest.undampedCcmMatrix);
            LOGD_ACCM("Adjust ccm by sat: %d, undampedCcmMatrix[0]: %f", hAccm->isReCal_, hAccm->accmRest.undampedCcmMatrix[0]);

            if (pCcm->lumaCCM.asym_enable) {
                int mid = CCM_CURVE_DOT_NUM_V2 >> 1;
                for (int i = 0; i < mid; i++) {
                    hAccm->ccmHwConf_v2.alp_y[i] =
                        fScale * pCcm->lumaCCM.y_alp_asym.y_alpha_left_curve[i];
                    hAccm->ccmHwConf_v2.alp_y[mid + i] =
                        fScale * pCcm->lumaCCM.y_alp_asym.y_alpha_right_curve[i];
                }
            } else {
                for (int i = 0; i < CCM_CURVE_DOT_NUM; i++) {  // set to ic  to do bit check
                    hAccm->ccmHwConf_v2.alp_y[i] = fScale * pCcm->lumaCCM.y_alp_sym.y_alpha_curve[i];
                }
                hAccm->ccmHwConf_v2.alp_y[CCM_CURVE_DOT_NUM] = 1024;
            }
        }
    }
    // 7) . Damping
    float dampCoef = (pCcm->TuningPara.damp_enable && (hAccm->count > 1 || hAccm->invarMode > 0)) ? hAccm->accmSwInfo.awbIIRDampCoef : 0;
    if (!hAccm->accmSwInfo.ccmConverged || hAccm->isReCal_) {
        ret = Damping(dampCoef,
                    hAccm->accmRest.undampedCcmMatrix, hAccm->ccmHwConf_v2.matrix,
                    hAccm->accmRest.undampedCcOffset, hAccm->ccmHwConf_v2.offs,
                    &hAccm->accmSwInfo.ccmConverged);
        hAccm->isReCal_ = true;
        LOGD_ACCM(
            "damping: %f, ccm coef[0]: %f->%f, ccm coef[8]: %f->%f",
            dampCoef, hAccm->accmRest.undampedCcmMatrix[0], hAccm->ccmHwConf_v2.matrix[0],
            hAccm->accmRest.undampedCcmMatrix[8], hAccm->ccmHwConf_v2.matrix[8]);
    }

    if (hAccm->update) {
        unsigned short enh_adj_en = 0;
        float enh_rat_max = 0;
        interpolation(pCcm->enhCCM.enh_ctl.gains, pCcm->enhCCM.enh_ctl.enh_adj_en, 9,
                    sensorGain, &enh_adj_en);
        if (enh_adj_en) {
            interpolation(pCcm->enhCCM.enh_ctl.gains, pCcm->enhCCM.enh_ctl.enh_rat_max, 9,
                            sensorGain, &enh_rat_max);
        }
        hAccm->isReCal_ = hAccm->isReCal_ ||
                            (enh_adj_en != hAccm->ccmHwConf_v2.enh_adj_en) ||
                            fabs(enh_rat_max - hAccm->ccmHwConf_v2.enh_rat_max) > DIVMIN;
    }

    LOGD_ACCM("final isReCal_ = %d \n", hAccm->isReCal_);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

XCamReturn AccmManualConfig(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    memcpy(hAccm->ccmHwConf_v2.matrix, hAccm->mCurAttV2.stManual.ccMatrix,
           sizeof(hAccm->mCurAttV2.stManual.ccMatrix));
    memcpy(hAccm->ccmHwConf_v2.offs, hAccm->mCurAttV2.stManual.ccOffsets,
           sizeof(hAccm->mCurAttV2.stManual.ccOffsets));
    hAccm->ccmHwConf_v2.highy_adj_en = hAccm->mCurAttV2.stManual.highy_adj_en;
    hAccm->ccmHwConf_v2.asym_adj_en  = hAccm->mCurAttV2.stManual.asym_enable;
    hAccm->ccmHwConf_v2.bound_bit    = hAccm->mCurAttV2.stManual.bound_pos_bit;
    hAccm->ccmHwConf_v2.right_bit    = hAccm->mCurAttV2.stManual.right_pos_bit;
    memcpy(hAccm->ccmHwConf_v2.alp_y, hAccm->mCurAttV2.stManual.y_alpha_curve,
           sizeof(hAccm->mCurAttV2.stManual.y_alpha_curve));

    memcpy(hAccm->ccmHwConf_v2.enh_rgb2y_para, hAccm->mCurAttV2.stManual.enh_rgb2y_para,
           sizeof(hAccm->mCurAttV2.stManual.enh_rgb2y_para));
    hAccm->ccmHwConf_v2.enh_adj_en  = hAccm->mCurAttV2.stManual.enh_adj_en;
    hAccm->ccmHwConf_v2.enh_rat_max = hAccm->mCurAttV2.stManual.enh_rat_max;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn AccmConfig(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOGD_ACCM("%s: byPass: %d  mode:%d updateAtt: %d \n", __FUNCTION__, hAccm->mCurAttV2.byPass,
              hAccm->mCurAttV2.mode, hAccm->updateAtt);
    if (hAccm->mCurAttV2.byPass != true && hAccm->accmSwInfo.grayMode != true) {
        hAccm->ccmHwConf_v2.ccmEnable = true;

        if (hAccm->mCurAttV2.mode == RK_AIQ_CCM_MODE_AUTO) {
            hAccm->update = JudgeCcmRes3aConverge(&hAccm->accmRest.res3a_info, &hAccm->accmSwInfo,
                                          hAccm->ccm_v2->control.gain_tolerance,
                                          hAccm->ccm_v2->control.wbgain_tolerance);
            hAccm->update       = hAccm->update || hAccm->calib_update; // wbgain/gain/calib changed
            LOGD_ACCM("%s: CCM update (gain/awbgain/calib): %d, CCM Converged: %d\n",
                __FUNCTION__, hAccm->update, hAccm->accmSwInfo.ccmConverged);
            if (hAccm->updateAtt || hAccm->update || (!hAccm->accmSwInfo.ccmConverged)) {
                AccmAutoConfig(hAccm);
                CCMV2PrintDBG(hAccm);
            }
        } else if (hAccm->mCurAttV2.mode == RK_AIQ_CCM_MODE_MANUAL) {
            if (hAccm->updateAtt) {
                AccmManualConfig(hAccm);
                hAccm->isReCal_ = true;
            }
        } else {
            LOGE_ACCM("%s: hAccm->mCurAttV2.mode(%d) is invalid \n", __FUNCTION__,
                      hAccm->mCurAttV2.mode);
        }

    } else {
        hAccm->ccmHwConf_v2.ccmEnable = false;
        // graymode/api/calib changed
        hAccm->isReCal_ = hAccm->isReCal_ || hAccm->updateAtt || hAccm->calib_update;
    }
    hAccm->updateAtt = false;
    hAccm->calib_update = false;
    hAccm->count = ((hAccm->count + 2) > (65536)) ? 2 : (hAccm->count + 1);

    CCMV2PrintReg(&hAccm->ccmHwConf_v2);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn ConfigbyCalib(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    const CalibDbV2_Ccm_Para_V32_t* calib_ccm = hAccm->ccm_v2;

#if RKAIQ_ACCM_ILLU_VOTE
        ReloadCCMCalibV2(hAccm, &calib_ccm->TuningPara);
#endif

    ret = pCcmMatrixAll_init(hAccm, &calib_ccm->TuningPara);
    if (hAccm->mCurAttV2.mode == RK_AIQ_CCM_MODE_AUTO)
        hAccm->mCurAttV2.byPass = !(calib_ccm->control.enable);

    hAccm->ccmHwConf_v2.asym_adj_en = calib_ccm->lumaCCM.asym_enable;

    if (calib_ccm->lumaCCM.asym_enable) {
        hAccm->ccmHwConf_v2.highy_adj_en = true;
        hAccm->ccmHwConf_v2.bound_bit    = calib_ccm->lumaCCM.y_alp_asym.bound_pos_bit;
        hAccm->ccmHwConf_v2.right_bit    = calib_ccm->lumaCCM.y_alp_asym.right_pos_bit;
        int mid                          = CCM_CURVE_DOT_NUM_V2 >> 1;
        for (int i = 0; i < mid; i++) {
            hAccm->ccmHwConf_v2.alp_y[i] = calib_ccm->lumaCCM.y_alp_asym.y_alpha_left_curve[i];
            hAccm->ccmHwConf_v2.alp_y[mid + i] =
                calib_ccm->lumaCCM.y_alp_asym.y_alpha_right_curve[i];
        }
    } else {
        hAccm->ccmHwConf_v2.highy_adj_en = calib_ccm->lumaCCM.y_alp_sym.highy_adj_en;
        hAccm->ccmHwConf_v2.bound_bit    = calib_ccm->lumaCCM.y_alp_sym.bound_pos_bit;
        hAccm->ccmHwConf_v2.right_bit    = hAccm->ccmHwConf_v2.bound_bit;
        memcpy(hAccm->ccmHwConf_v2.alp_y, calib_ccm->lumaCCM.y_alp_sym.y_alpha_curve,
               sizeof(calib_ccm->lumaCCM.y_alp_sym.y_alpha_curve));
        hAccm->ccmHwConf_v2.alp_y[CCM_CURVE_DOT_NUM] = 1024;
    }

    memcpy(hAccm->ccmHwConf_v2.rgb2y_para, calib_ccm->lumaCCM.rgb2y_para,
           sizeof(calib_ccm->lumaCCM.rgb2y_para));

    memcpy(hAccm->ccmHwConf_v2.enh_rgb2y_para, calib_ccm->enhCCM.enh_rgb2y_para,
           sizeof(calib_ccm->enhCCM.enh_rgb2y_para));

    hAccm->ccmHwConf_v2.enh_adj_en  = calib_ccm->enhCCM.enh_ctl.enh_adj_en[0];
    hAccm->ccmHwConf_v2.enh_rat_max = calib_ccm->enhCCM.enh_ctl.enh_rat_max[0];

    hAccm->accmSwInfo.ccmConverged = false;
    hAccm->calib_update            = true;

    clear_list(&hAccm->accmRest.problist);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

/**********************************
 *Update CCM CalibV2 Para
 *      Prepare init
 *      Mode change: reinit
 *      Res change: continue
 *      Calib change: continue
 ***************************************/
static XCamReturn UpdateCcmCalibV2ParaV2(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    bool config_calib = !!(hAccm->accmSwInfo.prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB);
    config_calib |= hAccm->isApiUpdateCalib;

    if (!config_calib) {
        return (ret);
    }

    ret = ConfigbyCalib(hAccm);
    hAccm->isApiUpdateCalib = false;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

XCamReturn AccmInit(accm_handle_t* hAccm, const CamCalibDbV2Context_t* calibv2) {
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret               = XCAM_RETURN_NO_ERROR;

    if (calibv2 == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    const CalibDbV2_Ccm_Para_V32_t* calib_ccm =
        (CalibDbV2_Ccm_Para_V32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)calibv2, ccm_calib_v2));
    if (calib_ccm == NULL) return XCAM_RETURN_ERROR_MEM;

    *hAccm                       = (accm_context_t*)malloc(sizeof(accm_context_t));
    accm_context_t* accm_context = *hAccm;
    memset(accm_context, 0, sizeof(accm_context_t));

    accm_context->accmSwInfo.sensorGain     = 1.0;
    accm_context->accmSwInfo.awbIIRDampCoef = 0;
    accm_context->accmSwInfo.varianceLuma   = 255;
    accm_context->accmSwInfo.awbConverged   = false;

    accm_context->accmSwInfo.awbGain[0]     = 1;
    accm_context->accmSwInfo.awbGain[1]     = 1;

    accm_context->accmRest.res3a_info.sensorGain = 1.0;
    accm_context->accmRest.res3a_info.awbGain[0] = 1.0;
    accm_context->accmRest.res3a_info.awbGain[1] = 1.0;

    accm_context->count = 0;
    accm_context->isReCal_ = 1;
    accm_context->invarMode = 1;

    accm_context->accmSwInfo.prepare_type =
        RK_AIQ_ALGO_CONFTYPE_UPDATECALIB | RK_AIQ_ALGO_CONFTYPE_NEEDRESET;

    // todo whm --- CalibDbV2_Ccm_Para_V2
    accm_context->ccm_v2 = calib_ccm;
    accm_context->isApiUpdateCalib = false;
    accm_context->ApiCalib_v2.enhCCM = calib_ccm->enhCCM;
    accm_context->ApiCalib_v2.TuningPara.illu_estim = calib_ccm->TuningPara.illu_estim;
    accm_context->mCurAttV2.mode = RK_AIQ_CCM_MODE_AUTO;
#if RKAIQ_ACCM_ILLU_VOTE
    INIT_LIST_HEAD(&accm_context->accmRest.dominateIlluList);
#endif
    INIT_LIST_HEAD(&accm_context->accmRest.problist);
    ret                  = UpdateCcmCalibV2ParaV2(accm_context);


    for (int i = 0; i < RK_AIQ_ACCM_COLOR_GAIN_NUM; i++) {
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[i] = 1;
        accm_context->mCurAttV2.stAuto.color_inhibition.level[i]      = 0;
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[i] = 1;
        accm_context->mCurAttV2.stAuto.color_saturation.level[i]      = 50;
    }
    accm_context->accmRest.fScale = 1;
    accm_context->accmRest.color_inhibition_level = 0;
    accm_context->accmRest.color_saturation_level = 100;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

XCamReturn AccmRelease(accm_handle_t hAccm) {
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if RKAIQ_ACCM_ILLU_VOTE
    clear_list(&hAccm->accmRest.dominateIlluList);
#endif
    clear_list(&hAccm->accmRest.problist);
    free(hAccm);
    hAccm = NULL;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

// todo whm
XCamReturn AccmPrepare(accm_handle_t hAccm) {
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = UpdateCcmCalibV2ParaV2(hAccm);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn AccmPreProc(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}
XCamReturn AccmProcessing(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

RKAIQ_END_DECLARE
