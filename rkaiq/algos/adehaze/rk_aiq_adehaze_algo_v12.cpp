/*
 * rk_aiq_adehaze_algo_v12.cpp
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
#include "rk_aiq_adehaze_algo_v12.h"
#include <string.h>
#include "xcam_log.h"

int DehazeLinearInterpV12(const float* pX, const float* pY, float posx, int BitInt, int BitFloat,
                          int XSize) {
    int index;
    float yOut     = 0.0;
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    if (posx >= pX[XSize - 1]) {
        yOut = pY[XSize - 1];
    } else if (posx <= pX[0]) {
        yOut = pY[0];
    } else {
        index = 0;
        while ((posx >= pX[index]) && (index < XSize)) {
            index++;
        }
        index -= 1;
        yOut = ((pY[index + 1] - pY[index]) / (pX[index + 1] - pX[index]) * (posx - pX[index])) +
               pY[index];
    }

    yOutInt = LIMIT_VALUE((int)(yOut * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

void DehazeHistWrTableInterpV12(const HistWr_t* pCurveIn, mManual_curve_t* pCurveOut, float posx) {
    int i       = 0;
    float ratio = 1.0;

    if (posx < pCurveIn->manual_curve[0].CtrlData) {
        for (int i = 0; i < DHAZ_V12_HIST_WR_CURVE_NUM; i++) {
            pCurveOut->curve_x[i] = pCurveIn->manual_curve[0].curve_x[i];
            pCurveOut->curve_y[i] = pCurveIn->manual_curve[0].curve_y[i];
        }
    } else if (posx >= pCurveIn->manual_curve[12].CtrlData) {
        for (int i = 0; i < DHAZ_V12_HIST_WR_CURVE_NUM; i++) {
            pCurveOut->curve_x[i] = pCurveIn->manual_curve[12].curve_x[i];
            pCurveOut->curve_y[i] = pCurveIn->manual_curve[12].curve_y[i];
        }
    } else {
        for (int i = 0; i < DHAZ_CTRL_DATA_STEP_MAX; i++) {
            if (posx >= pCurveIn->manual_curve[i].CtrlData &&
                posx < pCurveIn->manual_curve[i + 1].CtrlData) {
                if ((pCurveIn->manual_curve[i + 1].CtrlData - pCurveIn->manual_curve[i].CtrlData) !=
                    0)
                    ratio = (posx - pCurveIn->manual_curve[i].CtrlData) /
                            (pCurveIn->manual_curve[i + 1].CtrlData -
                             pCurveIn->manual_curve[i].CtrlData);
                else
                    LOGE_ADEHAZE("Dehaze zero in %s(%d) \n", __func__, __LINE__);

                for (int j = 0; j < DHAZ_V12_HIST_WR_CURVE_NUM; j++) {
                    pCurveOut->curve_x[j] = ratio * (pCurveIn->manual_curve[i + 1].curve_x[j] -
                                                     pCurveIn->manual_curve[i].curve_x[j]) +
                                            pCurveIn->manual_curve[i].curve_x[j];
                    pCurveOut->curve_y[j] = ratio * (pCurveIn->manual_curve[i + 1].curve_y[j] -
                                                     pCurveIn->manual_curve[i].curve_y[j]) +
                                            pCurveIn->manual_curve[i].curve_y[j];
                }
                break;
            } else
                continue;
        }
    }

    // check curve_x
    for (int i = 1; i < DHAZ_CTRL_DATA_STEP_MAX; i++) {
        if (!(pCurveOut->curve_x[i] % HSIT_WR_MIN_STEP))
            continue;
        else {
            int orig_x            = pCurveOut->curve_x[i];
            pCurveOut->curve_x[i] = HSIT_WR_MIN_STEP * (pCurveOut->curve_x[i] / HSIT_WR_MIN_STEP);
            if (orig_x != 0) {
                pCurveOut->curve_y[i] = pCurveOut->curve_y[i - 1] +
                                        (pCurveOut->curve_x[i] - pCurveOut->curve_x[i - 1]) *
                                            (pCurveOut->curve_y[i] - pCurveOut->curve_y[i - 1]) /
                                            (orig_x - pCurveOut->curve_x[i - 1]);
            }
            continue;
        }
    }

#if 0
    LOGD_ADEHAZE(
        "%s hist_wr.curve_x[0~11]:0x%x 0x%x 0x%x "
        "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
        __func__, pCurveOut->curve_x[0], pCurveOut->curve_x[1], pCurveOut->curve_x[2],
        pCurveOut->curve_x[3], pCurveOut->curve_x[4], pCurveOut->curve_x[5], pCurveOut->curve_x[6],
        pCurveOut->curve_x[7], pCurveOut->curve_x[8], pCurveOut->curve_x[9], pCurveOut->curve_x[10],
        pCurveOut->curve_x[11]);
    LOGD_ADEHAZE(
        "%s hist_wr.curve_y[0~11]:0x%x 0x%x 0x%x "
        "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
        __func__, pCurveOut->curve_y[0], pCurveOut->curve_y[1], pCurveOut->curve_y[2],
        pCurveOut->curve_y[3], pCurveOut->curve_y[4], pCurveOut->curve_y[5], pCurveOut->curve_y[6],
        pCurveOut->curve_y[7], pCurveOut->curve_y[8], pCurveOut->curve_y[9], pCurveOut->curve_y[10],
        pCurveOut->curve_y[11]);
#endif
}

float LinearInterpV12(const float* pX, const float* pY, float posx, int XSize) {
    int index;
    float yOut = 0;

    if (posx >= pX[XSize - 1]) {
        yOut = pY[XSize - 1];
    } else if (posx <= pX[0]) {
        yOut = pY[0];
    } else {
        index = 0;
        while ((posx >= pX[index]) && (index < XSize)) {
            index++;
        }
        index -= 1;
        yOut = ((pY[index + 1] - pY[index]) / (pX[index + 1] - pX[index]) * (posx - pX[index])) +
               pY[index];
    }

    return yOut;
}

int ClipValueV12(float posx, int BitInt, int BitFloat) {
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    yOutInt = LIMIT_VALUE((int)(posx * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

XCamReturn TransferHistWr2Res(RkAiqAdehazeProcResult_t* pProcRes, mManual_curve_t* pCurve) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // check curve_x
    for (int i = 0; i < DHAZ_V12_HIST_WR_CURVE_NUM - 2; i++) {
        if (!(pCurve->curve_x[i] % HSIT_WR_MIN_STEP) &&
            !(pCurve->curve_x[i + 1] % HSIT_WR_MIN_STEP)) {
            if (pCurve->curve_x[i] < pCurve->curve_x[i + 1])
                continue;
            else {
                LOGE_ADEHAZE("%s hist_wr.manu_curve.curve_x[%d] is samller than curve_x[%d]\n",
                             __func__, i + 1, i);
                return XCAM_RETURN_ERROR_PARAM;
            }
        } else {
            LOGE_ADEHAZE("%s hist_wr.manu_curve.curve_x[%d] can not be divided by 16\n", __func__,
                         i);
            return XCAM_RETURN_ERROR_PARAM;
        }
    }
    if (!(!(pCurve->curve_x[16] % HSIT_WR_MIN_STEP) || pCurve->curve_x[16] == HSIT_WR_X_MAX)) {
        LOGE_ADEHAZE("%s hist_wr.manu_curve.curve_x[12] can not be divided by 16\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int step      = 0;
    int add_knots = 0;
    int k         = 0;
    for (int i = 0; i < DHAZ_V12_HIST_WR_CURVE_NUM - 2; i++) {
        pProcRes->ProcResV12.hist_wr[k] = pCurve->curve_y[i];
        pProcRes->ProcResV12.hist_wr[k] =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.hist_wr[k], BIT_10_MAX);
        step      = (pCurve->curve_x[i + 1] - pCurve->curve_x[i]) / HSIT_WR_MIN_STEP;
        add_knots = step;
        for (int j = 1; step; step--) {
            pProcRes->ProcResV12.hist_wr[k + j] =
                pCurve->curve_y[i] + HSIT_WR_MIN_STEP * j *
                                         (pCurve->curve_y[i + 1] - pCurve->curve_y[i]) /
                                         (pCurve->curve_x[i + 1] - pCurve->curve_x[i]);
            pProcRes->ProcResV12.hist_wr[k + j] =
                LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.hist_wr[k + j], BIT_10_MAX);
            j++;
        }
        k += add_knots;
    }

    step = (1024 - pCurve->curve_x[15]) / HSIT_WR_MIN_STEP;
    for (int j = 1; step; step--) {
        pProcRes->ProcResV12.hist_wr[k + j] =
            pCurve->curve_y[15] +
            HSIT_WR_MIN_STEP * j * (1024 - pCurve->curve_y[15]) / (1024 - pCurve->curve_x[15]);
        pProcRes->ProcResV12.hist_wr[k + j] =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.hist_wr[k + j], BIT_10_MAX);
        j++;
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

void calc_cdf(int* hist, int* cdf, int bin_num, float clim, uint16_t vmin, uint16_t vmax) {
    int sum = 0;
    int i;
    int fix_clim;
    int extra = 0;
    int add;

    if (bin_num <= 0) return;

    for (i = 0; i < bin_num; ++i) {
        hist[i] = hist[i] * 8;
        sum += hist[i];
    }

    fix_clim = ((int)(clim * sum)) / bin_num;

    for (i = 0; i < bin_num; ++i) {
        if (hist[i] > fix_clim) {
            extra += hist[i] - fix_clim;
            hist[i] = fix_clim;
        }
    }

    add = (extra + bin_num / 2) / bin_num;

    for (i = 0; i < bin_num; ++i) {
        hist[i] += add;
    }

    cdf[0] = hist[0];

    for (i = 1; i < bin_num; ++i) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    if (cdf[bin_num - 1] > 0) {
        for (i = 0; i < bin_num; ++i) {
            cdf[i] = cdf[i] * (vmax - vmin) / cdf[bin_num - 1] + vmin;
        }
    } else {
        for (i = 0; i < bin_num; ++i) {
            cdf[i] = i * (vmax - vmin) / (bin_num - 1) + vmin;
        }
    }
}

XCamReturn TransferHistWrSemiAtuos2Res(RkAiqAdehazeProcResult_t* pProcRes,
                                       mhist_wr_semiauto_t* pSemiAutoCurve,
                                       dehaze_stats_v12_t* pStats, bool stats_true) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    if (!stats_true) return XCAM_RETURN_BYPASS;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int i;
    int hist[64];
    int cdf_out[64];
    int dark_idx = 1;
    int mean     = 0;
    static bool first_time = true;

    if (first_time) {
        for (i = 0; i < 63; ++i) {
            pProcRes->ProcResV12.hist_wr[i] = (i + 1) * 16;
        }
        pProcRes->ProcResV12.hist_wr[i] = 1023;
        first_time                      = false;
        return XCAM_RETURN_NO_ERROR;
    }

    if (pStats->h_rgb_iir[63] != 1023) {
        LOGW_ADEHAZE("invalid cdf input, using normalize output\n");
        return XCAM_RETURN_BYPASS;
    }

    for (i = 0; i < 64; ++i) {
        hist[i] = (pStats->h_rgb_iir[i / 2] >> ((i % 2) * 16)) & 0xffff;
    }

    for (i = 63; i > 0; --i) {
        hist[i] = LIMIT_VALUE(pStats->h_rgb_iir[i] - pStats->h_rgb_iir[i - 1], 1023, 0);
        mean += (i + 1) * 16 * hist[i];
    }

    hist[0] = pStats->h_rgb_iir[0];
    mean += 16 * hist[0];
    mean = mean >> 10;
    pSemiAutoCurve->dark_th = MAX(1024 - mean, pSemiAutoCurve->dark_th);
    dark_idx                = (int(pSemiAutoCurve->dark_th + 8.0f) / 16.0f);
    dark_idx                = LIMIT_VALUE(dark_idx, 64, 1);

    calc_cdf(hist, cdf_out, dark_idx, pSemiAutoCurve->clim0, 0, dark_idx * 16);
    calc_cdf(hist + dark_idx, cdf_out + dark_idx, 64 - dark_idx, pSemiAutoCurve->clim1,
             dark_idx * 16, 1023);

    for (i = 0; i < 64; ++i) {
        if (i < dark_idx)
            cdf_out[i] = cdf_out[i] < (i + 1) * 16 ? (i + 1) * 16 : cdf_out[i];
        else if (cdf_out[i] < cdf_out[i - 1])
            cdf_out[i] = cdf_out[i - 1];
        pProcRes->ProcResV12.hist_wr[i] =
            (7 * pProcRes->ProcResV12.hist_wr[i] + cdf_out[i] + 4) / 8;
        pProcRes->ProcResV12.hist_wr[i] =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.hist_wr[i], 1023);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

void stManuGetDehazeParamsV12(mDehazeAttrV12_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                              int rawWidth, int rawHeight, unsigned int MDehazeStrth) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV12.air_lc_en =
        pStManu->dehaze_setting.air_lc_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    pProcRes->ProcResV12.dc_min_th =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.dc_min_th, 8, 0);
    pProcRes->ProcResV12.dc_max_th =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.dc_max_th, 8, 0);
    pProcRes->ProcResV12.yhist_th = ClipValueV12(pStManu->dehaze_setting.DehazeData.yhist_th, 8, 0);
    pProcRes->ProcResV12.yblk_th  = int(pStManu->dehaze_setting.DehazeData.yblk_th *
                                        ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16));
    pProcRes->ProcResV12.dark_th  = ClipValueV12(pStManu->dehaze_setting.DehazeData.dark_th, 8, 0);
    pProcRes->ProcResV12.bright_min =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.bright_min, 8, 0);
    pProcRes->ProcResV12.bright_max =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.bright_max, 8, 0);
    pProcRes->ProcResV12.wt_max  = ClipValueV12(pStManu->dehaze_setting.DehazeData.wt_max, 0, 8);
    pProcRes->ProcResV12.air_min = ClipValueV12(pStManu->dehaze_setting.DehazeData.air_min, 8, 0);
    pProcRes->ProcResV12.air_max = ClipValueV12(pStManu->dehaze_setting.DehazeData.air_max, 8, 0);
    pProcRes->ProcResV12.tmax_base =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.tmax_base, 8, 0);
    pProcRes->ProcResV12.tmax_off =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.tmax_off, 0, 10);
    pProcRes->ProcResV12.tmax_max =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.tmax_max, 0, 10);
    pProcRes->ProcResV12.cfg_wt  = ClipValueV12(pStManu->dehaze_setting.DehazeData.cfg_wt, 0, 8);
    pProcRes->ProcResV12.cfg_air = ClipValueV12(pStManu->dehaze_setting.DehazeData.cfg_air, 8, 0);
    pProcRes->ProcResV12.cfg_tmax =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.cfg_tmax, 0, 10);
    pProcRes->ProcResV12.range_sima =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.range_sigma, 0, 8);
    pProcRes->ProcResV12.space_sigma_cur =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.space_sigma_cur, 0, 8);
    pProcRes->ProcResV12.space_sigma_pre =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.space_sigma_pre, 0, 8);
    pProcRes->ProcResV12.bf_weight =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.bf_weight, 0, 8);
    pProcRes->ProcResV12.dc_weitcur =
        ClipValueV12(pStManu->dehaze_setting.DehazeData.dc_weitcur, 0, 8);
    pProcRes->ProcResV12.stab_fnum      = ClipValueV12(pStManu->dehaze_setting.stab_fnum, 5, 0);
    if (pStManu->dehaze_setting.sigma)
        pProcRes->ProcResV12.iir_sigma =
            LIMIT_VALUE(int(256.0f / pStManu->dehaze_setting.sigma), 255, 0);
    else
        pProcRes->ProcResV12.iir_sigma = 0x1;
    if (pStManu->dehaze_setting.wt_sigma >= 0.0f)
        pProcRes->ProcResV12.iir_wt_sigma =
            LIMIT_VALUE(int(1024.0f / (8.0f * pStManu->dehaze_setting.wt_sigma + 0.5f)), 0x7ff, 0);
    else
        pProcRes->ProcResV12.iir_wt_sigma = 0x7ff;
    if (pStManu->dehaze_setting.air_sigma)
        pProcRes->ProcResV12.iir_air_sigma =
            LIMIT_VALUE(int(1024.0f / pStManu->dehaze_setting.air_sigma), 255, 0);
    else
        pProcRes->ProcResV12.iir_air_sigma = 0x8;
    if (pStManu->dehaze_setting.tmax_sigma)
        pProcRes->ProcResV12.iir_tmax_sigma =
            LIMIT_VALUE(int(1.0f / pStManu->dehaze_setting.tmax_sigma), 0x7ff, 0);
    else
        pProcRes->ProcResV12.iir_tmax_sigma = 0x5f;
    pProcRes->ProcResV12.iir_pre_wet =
        LIMIT_VALUE(int(pStManu->dehaze_setting.pre_wet - 1.0f), 15, 0);
    pProcRes->ProcResV12.gaus_h0        = DEHAZE_GAUS_H4;
    pProcRes->ProcResV12.gaus_h1        = DEHAZE_GAUS_H1;
    pProcRes->ProcResV12.gaus_h2        = DEHAZE_GAUS_H0;

    // add for rk_aiq_uapi2_setMDehazeStrth
    if (MDehazeStrth != DEHAZE_DEFAULT_LEVEL) {
        pProcRes->ProcResV12.cfg_alpha = BIT_8_MAX;
        unsigned int level_diff        = MDehazeStrth > DEHAZE_DEFAULT_LEVEL
                                      ? (MDehazeStrth - DEHAZE_DEFAULT_LEVEL)
                                      : (DEHAZE_DEFAULT_LEVEL - MDehazeStrth);
        bool level_up = MDehazeStrth > DEHAZE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV12.cfg_wt += level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV12.cfg_air += level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV12.cfg_tmax += level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        } else {
            pProcRes->ProcResV12.cfg_wt =
                level_diff * DEHAZE_DEFAULT_CFG_WT_STEP > pProcRes->ProcResV12.cfg_wt
                    ? 0
                    : (pProcRes->ProcResV12.cfg_wt - level_diff * DEHAZE_DEFAULT_CFG_WT_STEP);
            pProcRes->ProcResV12.cfg_air =
                level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP > pProcRes->ProcResV12.cfg_air
                    ? 0
                    : (pProcRes->ProcResV12.cfg_air - level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP);
            pProcRes->ProcResV12.cfg_tmax =
                level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP > pProcRes->ProcResV12.cfg_tmax
                    ? 0
                    : (pProcRes->ProcResV12.cfg_tmax - level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP);
        }
        pProcRes->ProcResV12.cfg_wt = LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.cfg_wt, BIT_8_MAX);
        pProcRes->ProcResV12.cfg_tmax =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.cfg_tmax, BIT_10_MAX);
    }

    if (pProcRes->ProcResV12.dc_en && !(pProcRes->ProcResV12.enhance_en)) {
        if (pProcRes->ProcResV12.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:1 cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__,
                         pProcRes->ProcResV12.cfg_air / 1.0f,
                         pProcRes->ProcResV12.cfg_tmax / 1023.0f,
                         pProcRes->ProcResV12.cfg_wt / 255.0f);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x255 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n",
                         __func__, pProcRes->ProcResV12.cfg_air, pProcRes->ProcResV12.cfg_tmax,
                         pProcRes->ProcResV12.cfg_wt);
        } else if (pProcRes->ProcResV12.cfg_alpha == 0) {
            LOGD_ADEHAZE("%s cfg_alpha:0 air_max:%f air_min:%f tmax_base:%f wt_max:%f\n", __func__,
                         pProcRes->ProcResV12.air_max / 1.0f, pProcRes->ProcResV12.air_min / 1.0f,
                         pProcRes->ProcResV12.tmax_base / 1.0f,
                         pProcRes->ProcResV12.wt_max / 255.0f);
            LOGD_ADEHAZE(
                "%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n",
                __func__, pProcRes->ProcResV12.air_max, pProcRes->ProcResV12.air_min,
                pProcRes->ProcResV12.tmax_base, pProcRes->ProcResV12.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetEnhanceParamsV12(mDehazeAttrV12_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                               unsigned int MEnhanceStrth, unsigned int MEnhanceChromeStrth) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool level_up           = false;
    unsigned int level_diff = 0;

    pProcRes->ProcResV12.enhance_value =
        ClipValueV12(pStManu->enhance_setting.EnhanceData.enhance_value, 4, 10);
    pProcRes->ProcResV12.enhance_chroma =
        ClipValueV12(pStManu->enhance_setting.EnhanceData.enhance_chroma, 4, 10);

    for (int i = 0; i < DHAZ_V12_ENHANCE_CRUVE_NUM; i++)
        pProcRes->ProcResV12.enh_curve[i] =
            (int)(pStManu->enhance_setting.EnhanceData.enhance_curve[i]);

    // dehaze v12 add
    pProcRes->ProcResV12.color_deviate_en = pStManu->enhance_setting.color_deviate_en;
    pProcRes->ProcResV12.enh_luma_en      = pStManu->enhance_setting.enh_luma_en;
    for (int i = 0; i < DHAZ_V12_ENH_LUMA_NUM; i++)
        pProcRes->ProcResV12.enh_luma[i] =
            ClipValueV12(pStManu->enhance_setting.EnhanceData.enh_luma[i], 4, 6);

    // add for rk_aiq_uapi2_setMEnhanceStrth
    if (MEnhanceStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceStrth);
        level_up = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL;
        if (pProcRes->ProcResV12.enh_luma_en) {
            if (level_up) {
                for (int j = 0; j < DHAZ_V12_ENH_LUMA_NUM; j++) {
                    pProcRes->ProcResV12.enh_luma[j] += level_diff * ENH_LUMA_DEFAULT_STEP;
                    pProcRes->ProcResV12.enh_luma[j] =
                        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.enh_luma[j], BIT_10_MAX);
                }
            } else {
                for (int j = 0; j < DHAZ_V12_ENH_LUMA_NUM; j++) {
                    pProcRes->ProcResV12.enh_luma[j] =
                        level_diff * ENH_LUMA_DEFAULT_STEP > pProcRes->ProcResV12.enh_luma[j]
                            ? 0
                            : (pProcRes->ProcResV12.enh_luma[j] -
                               level_diff * ENH_LUMA_DEFAULT_STEP);
                    pProcRes->ProcResV12.enh_luma[j] =
                        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.enh_luma[j], BIT_10_MAX);
                }
            }
        } else {
            if (level_up) {
                pProcRes->ProcResV12.enhance_value += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
            } else {
                pProcRes->ProcResV12.enhance_value =
                    level_diff * ENHANCE_VALUE_DEFAULT_STEP > pProcRes->ProcResV12.enhance_value
                        ? 0
                        : (pProcRes->ProcResV12.enhance_value -
                           level_diff * ENHANCE_VALUE_DEFAULT_STEP);
            }
            pProcRes->ProcResV12.enhance_value =
                LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.enhance_value, BIT_14_MAX);
        }
    }

    // add for rk_aiq_uapi2_setMEnhanceChromeStrth
    if (MEnhanceChromeStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceChromeStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceChromeStrth);
        level_up = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV12.enhance_chroma += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV12.enhance_chroma =
                level_diff * ENHANCE_VALUE_DEFAULT_STEP > pProcRes->ProcResV12.enhance_chroma
                    ? 0
                    : (pProcRes->ProcResV12.enhance_chroma -
                       level_diff * ENHANCE_VALUE_DEFAULT_STEP);
        }
        pProcRes->ProcResV12.enhance_chroma =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.enhance_chroma, BIT_14_MAX);
    }

    if (pProcRes->ProcResV12.dc_en && pProcRes->ProcResV12.enhance_en) {
        LOGD_ADEHAZE("%s color_deviate_en:%d enh_luma_en:%d\n", __func__,
                     pProcRes->ProcResV12.color_deviate_en, pProcRes->ProcResV12.enh_luma_en);
        LOGD_ADEHAZE("%s enhance_value:%f enhance_chroma:%f\n", __func__,
                     pStManu->enhance_setting.EnhanceData.enhance_value / 1024.0f,
                     pStManu->enhance_setting.EnhanceData.enhance_chroma / 1024.0f);
        if (pProcRes->ProcResV12.enh_luma_en) {
            LOGD_ADEHAZE(
                "%s enh_luma[0~7]:%f %f %f %f %f %f %f %f\n", __func__,
                pProcRes->ProcResV12.enh_luma[0] / 64.0f, pProcRes->ProcResV12.enh_luma[1] / 64.0f,
                pProcRes->ProcResV12.enh_luma[2] / 64.0f, pProcRes->ProcResV12.enh_luma[3] / 64.0f,
                pProcRes->ProcResV12.enh_luma[4] / 64.0f, pProcRes->ProcResV12.enh_luma[5] / 64.0f,
                pProcRes->ProcResV12.enh_luma[6] / 64.0f, pProcRes->ProcResV12.enh_luma[7] / 64.0f);
        }
        LOGD_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     pProcRes->ProcResV12.enhance_value, pProcRes->ProcResV12.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetHistParamsV12(mDehazeAttrV12_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                            dehaze_stats_v12_t* pStats, bool stats_true) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV12.hpara_en =
        pStManu->hist_setting.hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    // clip hpara_en
    pProcRes->ProcResV12.hpara_en =
        pProcRes->ProcResV12.dc_en ? pProcRes->ProcResV12.hpara_en : FUNCTION_ENABLE;

    pProcRes->ProcResV12.hist_gratio =
        ClipValueV12(pStManu->hist_setting.HistData.hist_gratio, 0, 8);
    pProcRes->ProcResV12.hist_th_off =
        ClipValueV12(pStManu->hist_setting.HistData.hist_th_off, 8, 0);
    pProcRes->ProcResV12.hist_k     = ClipValueV12(pStManu->hist_setting.HistData.hist_k, 3, 2);
    pProcRes->ProcResV12.hist_min   = ClipValueV12(pStManu->hist_setting.HistData.hist_min, 1, 8);
    pProcRes->ProcResV12.cfg_gratio = ClipValueV12(pStManu->hist_setting.HistData.cfg_gratio, 5, 8);
    pProcRes->ProcResV12.hist_scale = ClipValueV12(pStManu->hist_setting.HistData.hist_scale, 5, 8);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pStManu->hist_setting.hist_wr.mode == HIST_WR_MANUAL) {
        ret = TransferHistWr2Res(pProcRes, &pStManu->hist_setting.hist_wr.manual_curve);
        if (ret == XCAM_RETURN_NO_ERROR)
            pProcRes->ProcResV12.soft_wr_en = FUNCTION_ENABLE;
        else
            pProcRes->ProcResV12.soft_wr_en = FUNCTION_DISABLE;
    } else if (pStManu->hist_setting.hist_wr.mode == HIST_WR_AUTO) {
        pProcRes->ProcResV12.soft_wr_en = FUNCTION_DISABLE;
    } else if (pStManu->hist_setting.hist_wr.mode == HIST_WR_SEMIAUTO) {
        ret = TransferHistWrSemiAtuos2Res(pProcRes, &pStManu->hist_setting.hist_wr.semiauto_curve,
                                          pStats, stats_true);
        if (ret == XCAM_RETURN_NO_ERROR)
            pProcRes->ProcResV12.soft_wr_en = FUNCTION_ENABLE;
        else
            pProcRes->ProcResV12.soft_wr_en = FUNCTION_DISABLE;
    }

    if (pProcRes->ProcResV12.hist_en) {
        LOGD_ADEHAZE(
            "%s cfg_alpha:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f "
            "hist_min:%f hist_scale:%f cfg_gratio:%f\n",
            __func__, pProcRes->ProcResV12.cfg_alpha / 255.0f, pProcRes->ProcResV12.hpara_en,
            pProcRes->ProcResV12.hist_gratio / 255.0f, pProcRes->ProcResV12.hist_th_off / 1.0f,
            pProcRes->ProcResV12.hist_k / 4.0f, pProcRes->ProcResV12.hist_min / 256.0f,
            pProcRes->ProcResV12.hist_scale / 256.0f, pProcRes->ProcResV12.cfg_gratio / 256.0f);
        LOGD_ADEHAZE(
            "%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x "
            "hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n",
            __func__, pProcRes->ProcResV12.cfg_alpha, pProcRes->ProcResV12.hist_gratio,
            pProcRes->ProcResV12.hist_th_off, pProcRes->ProcResV12.hist_k,
            pProcRes->ProcResV12.hist_min, pProcRes->ProcResV12.hist_scale,
            pProcRes->ProcResV12.cfg_gratio);
        LOGD_ADEHAZE("%s soft_wr_en:%d hist_wr.mode:%d\n", __func__,
                     pProcRes->ProcResV12.soft_wr_en, pStManu->hist_setting.hist_wr.mode);
        if (pProcRes->ProcResV12.soft_wr_en) {
            LOGD_ADEHAZE(
                "%s hist_wr[0~9]:0x%x 0x%x 0x%x "
                "0x%x 0x%x 0x%x\n",
                __func__, pProcRes->ProcResV12.hist_wr[0], pProcRes->ProcResV12.hist_wr[1],
                pProcRes->ProcResV12.hist_wr[2], pProcRes->ProcResV12.hist_wr[3],
                pProcRes->ProcResV12.hist_wr[4], pProcRes->ProcResV12.hist_wr[5],
                pProcRes->ProcResV12.hist_wr[6], pProcRes->ProcResV12.hist_wr[7],
                pProcRes->ProcResV12.hist_wr[8], pProcRes->ProcResV12.hist_wr[9]);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeParamsV12(CalibDbDehazeV12_t* pCalibV12, RkAiqAdehazeProcResult_t* pProcRes,
                        int rawWidth, int rawHeight, unsigned int MDehazeStrth, float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV12.air_lc_en =
        pCalibV12->dehaze_setting.air_lc_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    pProcRes->ProcResV12.dc_min_th = DehazeLinearInterpV12(
                                         pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                         pCalibV12->dehaze_setting.DehazeData.dc_min_th, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.dc_max_th = DehazeLinearInterpV12(
                                         pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                         pCalibV12->dehaze_setting.DehazeData.dc_max_th, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.yhist_th = DehazeLinearInterpV12(
                                        pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                        pCalibV12->dehaze_setting.DehazeData.yhist_th, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.yblk_th = LinearInterpV12(pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                   pCalibV12->dehaze_setting.DehazeData.yblk_th,
                                   CtrlValue, DHAZ_CTRL_DATA_STEP_MAX) *
                                   ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16);
    pProcRes->ProcResV12.dark_th = DehazeLinearInterpV12(
                                       pCalibV12->dehaze_setting.DehazeData.CtrlData, pCalibV12->dehaze_setting.DehazeData.dark_th,
                                       CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.bright_min = DehazeLinearInterpV12(
                                          pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                          pCalibV12->dehaze_setting.DehazeData.bright_min, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.bright_max = DehazeLinearInterpV12(
                                          pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                          pCalibV12->dehaze_setting.DehazeData.bright_max, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.wt_max = DehazeLinearInterpV12(
                                      pCalibV12->dehaze_setting.DehazeData.CtrlData, pCalibV12->dehaze_setting.DehazeData.wt_max,
                                      CtrlValue, 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.air_min = DehazeLinearInterpV12(
                                       pCalibV12->dehaze_setting.DehazeData.CtrlData, pCalibV12->dehaze_setting.DehazeData.air_min,
                                       CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.air_max = DehazeLinearInterpV12(
                                       pCalibV12->dehaze_setting.DehazeData.CtrlData, pCalibV12->dehaze_setting.DehazeData.air_max,
                                       CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.tmax_base = DehazeLinearInterpV12(
                                         pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                         pCalibV12->dehaze_setting.DehazeData.tmax_base, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.tmax_off = DehazeLinearInterpV12(
                                        pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                        pCalibV12->dehaze_setting.DehazeData.tmax_off, CtrlValue, 0, 10, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.tmax_max = DehazeLinearInterpV12(
                                        pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                        pCalibV12->dehaze_setting.DehazeData.tmax_max, CtrlValue, 0, 10, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.cfg_wt = DehazeLinearInterpV12(
                                      pCalibV12->dehaze_setting.DehazeData.CtrlData, pCalibV12->dehaze_setting.DehazeData.cfg_wt,
                                      CtrlValue, 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.cfg_air = DehazeLinearInterpV12(
                                       pCalibV12->dehaze_setting.DehazeData.CtrlData, pCalibV12->dehaze_setting.DehazeData.cfg_air,
                                       CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.cfg_tmax = DehazeLinearInterpV12(
                                        pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                        pCalibV12->dehaze_setting.DehazeData.cfg_tmax, CtrlValue, 0, 10, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.range_sima = DehazeLinearInterpV12(
        pCalibV12->dehaze_setting.DehazeData.CtrlData,
        pCalibV12->dehaze_setting.DehazeData.range_sigma, CtrlValue, 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.space_sigma_cur =
        DehazeLinearInterpV12(pCalibV12->dehaze_setting.DehazeData.CtrlData,
                              pCalibV12->dehaze_setting.DehazeData.space_sigma_cur, CtrlValue, 0, 8,
                              DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.space_sigma_pre =
        DehazeLinearInterpV12(pCalibV12->dehaze_setting.DehazeData.CtrlData,
                              pCalibV12->dehaze_setting.DehazeData.space_sigma_pre, CtrlValue, 0, 8,
                              DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.bf_weight = DehazeLinearInterpV12(
                                         pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                         pCalibV12->dehaze_setting.DehazeData.bf_weight, CtrlValue, 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.dc_weitcur = DehazeLinearInterpV12(
                                          pCalibV12->dehaze_setting.DehazeData.CtrlData,
                                          pCalibV12->dehaze_setting.DehazeData.dc_weitcur, CtrlValue, 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.stab_fnum      = ClipValueV12(pCalibV12->dehaze_setting.stab_fnum, 5, 0);
    if (pCalibV12->dehaze_setting.sigma)
        pProcRes->ProcResV12.iir_sigma =
            LIMIT_VALUE(int(256.0f / pCalibV12->dehaze_setting.sigma), 255, 0);
    else
        pProcRes->ProcResV12.iir_sigma = 0x1;
    if (pCalibV12->dehaze_setting.wt_sigma >= 0.0f)
        pProcRes->ProcResV12.iir_wt_sigma = LIMIT_VALUE(
            int(1024.0f / (8.0f * pCalibV12->dehaze_setting.wt_sigma + 0.5f)), 0x7ff, 0);
    else
        pProcRes->ProcResV12.iir_wt_sigma = 0x7ff;
    if (pCalibV12->dehaze_setting.air_sigma)
        pProcRes->ProcResV12.iir_air_sigma =
            LIMIT_VALUE(int(1024.0f / pCalibV12->dehaze_setting.air_sigma), 255, 0);
    else
        pProcRes->ProcResV12.iir_air_sigma = 0x8;
    if (pCalibV12->dehaze_setting.tmax_sigma)
        pProcRes->ProcResV12.iir_tmax_sigma =
            LIMIT_VALUE(int(1.0f / pCalibV12->dehaze_setting.tmax_sigma), 0x7ff, 0);
    else
        pProcRes->ProcResV12.iir_tmax_sigma = 0x5f;
    pProcRes->ProcResV12.iir_pre_wet =
        LIMIT_VALUE(int(pCalibV12->dehaze_setting.pre_wet - 1.0f), 15, 0);
    pProcRes->ProcResV12.gaus_h0        = DEHAZE_GAUS_H4;
    pProcRes->ProcResV12.gaus_h1        = DEHAZE_GAUS_H1;
    pProcRes->ProcResV12.gaus_h2        = DEHAZE_GAUS_H0;

    // add for rk_aiq_uapi2_setMDehazeStrth
    if (MDehazeStrth != DEHAZE_DEFAULT_LEVEL) {
        pProcRes->ProcResV12.cfg_alpha = BIT_8_MAX;
        unsigned int level_diff        = MDehazeStrth > DEHAZE_DEFAULT_LEVEL
                                      ? (MDehazeStrth - DEHAZE_DEFAULT_LEVEL)
                                      : (DEHAZE_DEFAULT_LEVEL - MDehazeStrth);
        bool level_up = MDehazeStrth > DEHAZE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV12.cfg_wt += level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV12.cfg_air += level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV12.cfg_tmax += level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        } else {
            pProcRes->ProcResV12.cfg_wt =
                level_diff * DEHAZE_DEFAULT_CFG_WT_STEP > pProcRes->ProcResV12.cfg_wt
                    ? 0
                    : (pProcRes->ProcResV12.cfg_wt - level_diff * DEHAZE_DEFAULT_CFG_WT_STEP);
            pProcRes->ProcResV12.cfg_air =
                level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP > pProcRes->ProcResV12.cfg_air
                    ? 0
                    : (pProcRes->ProcResV12.cfg_air - level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP);
            pProcRes->ProcResV12.cfg_tmax =
                level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP > pProcRes->ProcResV12.cfg_tmax
                    ? 0
                    : (pProcRes->ProcResV12.cfg_tmax - level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP);
        }
        pProcRes->ProcResV12.cfg_wt = LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.cfg_wt, BIT_8_MAX);
        pProcRes->ProcResV12.cfg_tmax =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.cfg_tmax, BIT_10_MAX);
    }

    if (pProcRes->ProcResV12.dc_en && !(pProcRes->ProcResV12.enhance_en)) {
        if (pProcRes->ProcResV12.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:1 CtrlValue:%f cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__,
                         CtrlValue, pProcRes->ProcResV12.cfg_air / 1.0f,
                         pProcRes->ProcResV12.cfg_tmax / 1023.0f,
                         pProcRes->ProcResV12.cfg_wt / 255.0f);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x255 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n",
                         __func__, pProcRes->ProcResV12.cfg_air, pProcRes->ProcResV12.cfg_tmax,
                         pProcRes->ProcResV12.cfg_wt);
        } else if (pProcRes->ProcResV12.cfg_alpha == 0) {
            LOGD_ADEHAZE(
                "%s cfg_alpha:0 CtrlValue:%f air_max:%f air_min:%f tmax_base:%f wt_max:%f\n",
                __func__, CtrlValue, pProcRes->ProcResV12.air_max / 1.0f,
                pProcRes->ProcResV12.air_min / 1.0f, pProcRes->ProcResV12.tmax_base / 1.0f,
                pProcRes->ProcResV12.wt_max / 255.0f);
            LOGD_ADEHAZE(
                "%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n",
                __func__, pProcRes->ProcResV12.air_max, pProcRes->ProcResV12.air_min,
                pProcRes->ProcResV12.tmax_base, pProcRes->ProcResV12.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetEnhanceParamsV12(CalibDbDehazeV12_t* pCalibV12, RkAiqAdehazeProcResult_t* pProcRes,
                         unsigned int MEnhanceStrth, unsigned int MEnhanceChromeStrth,
                         float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool level_up           = false;
    unsigned int level_diff = 0;
    int i       = 0;
    float tmp   = 0.0f;
    float ratio = 1.0;

    if (CtrlValue < pCalibV12->enhance_setting.EnhanceData[0].CtrlData) {
        pProcRes->ProcResV12.enhance_value =
            ClipValueV12(pCalibV12->enhance_setting.EnhanceData[0].enhance_value, 4, 10);
        pProcRes->ProcResV12.enhance_chroma =
            ClipValueV12(pCalibV12->enhance_setting.EnhanceData[0].enhance_chroma, 4, 10);
        for (int i = 0; i < DHAZ_V12_ENHANCE_CRUVE_NUM; i++)
            pProcRes->ProcResV12.enh_curve[i] =
                (int)(pCalibV12->enhance_setting.EnhanceData[0].enhance_curve[i]);
        for (int i = 0; i < DHAZ_V12_ENH_LUMA_NUM; i++)
            pProcRes->ProcResV12.enh_luma[i] =
                ClipValueV12(pCalibV12->enhance_setting.EnhanceData[0].enh_luma[i], 4, 6);
    } else if (CtrlValue >= pCalibV12->enhance_setting.EnhanceData[12].CtrlData) {
        pProcRes->ProcResV12.enhance_value =
            ClipValueV12(pCalibV12->enhance_setting.EnhanceData[12].enhance_value, 4, 10);
        pProcRes->ProcResV12.enhance_chroma =
            ClipValueV12(pCalibV12->enhance_setting.EnhanceData[12].enhance_chroma, 4, 10);
        for (int i = 0; i < DHAZ_V12_ENHANCE_CRUVE_NUM; i++)
            pProcRes->ProcResV12.enh_curve[i] =
                (int)(pCalibV12->enhance_setting.EnhanceData[12].enhance_curve[i]);
        for (int i = 0; i < DHAZ_V12_ENH_LUMA_NUM; i++)
            pProcRes->ProcResV12.enh_luma[i] =
                ClipValueV12(pCalibV12->enhance_setting.EnhanceData[12].enh_luma[i], 4, 6);
    } else {
        for (int i = 0; i < DHAZ_CTRL_DATA_STEP_MAX; i++) {
            if (CtrlValue >= pCalibV12->enhance_setting.EnhanceData[i].CtrlData &&
                CtrlValue < pCalibV12->enhance_setting.EnhanceData[i + 1].CtrlData) {
                if ((pCalibV12->enhance_setting.EnhanceData[i + 1].CtrlData -
                     pCalibV12->enhance_setting.EnhanceData[i].CtrlData) != 0)
                    ratio = (CtrlValue - pCalibV12->enhance_setting.EnhanceData[i].CtrlData) /
                            (pCalibV12->enhance_setting.EnhanceData[i + 1].CtrlData -
                             pCalibV12->enhance_setting.EnhanceData[i].CtrlData);
                else
                    LOGE_ADEHAZE("Dehaze zero in %s(%d) \n", __func__, __LINE__);

                tmp = ratio * (pCalibV12->enhance_setting.EnhanceData[i + 1].enhance_value -
                               pCalibV12->enhance_setting.EnhanceData[i].enhance_value) +
                      pCalibV12->enhance_setting.EnhanceData[i].enhance_value;
                pProcRes->ProcResV12.enhance_value = ClipValueV12(tmp, 4, 10);
                tmp = ratio * (pCalibV12->enhance_setting.EnhanceData[i + 1].enhance_chroma -
                               pCalibV12->enhance_setting.EnhanceData[i].enhance_chroma) +
                      pCalibV12->enhance_setting.EnhanceData[i].enhance_chroma;
                pProcRes->ProcResV12.enhance_chroma = ClipValueV12(tmp, 4, 10);
                for (int j = 0; j < DHAZ_V12_ENHANCE_CRUVE_NUM; j++) {
                    pProcRes->ProcResV12.enh_curve[j] =
                        ratio * (pCalibV12->enhance_setting.EnhanceData[i + 1].enhance_curve[j] -
                                 pCalibV12->enhance_setting.EnhanceData[i].enhance_curve[j]) +
                        pCalibV12->enhance_setting.EnhanceData[i].enhance_curve[j];
                }
                for (int j = 0; j < DHAZ_V12_ENH_LUMA_NUM; j++) {
                    tmp = ratio * (pCalibV12->enhance_setting.EnhanceData[i + 1].enh_luma[j] -
                                   pCalibV12->enhance_setting.EnhanceData[i].enh_luma[j]) +
                          pCalibV12->enhance_setting.EnhanceData[i].enh_luma[j];
                    pProcRes->ProcResV12.enh_luma[j] = ClipValueV12(tmp, 4, 6);
                }
                break;
            } else
                continue;
        }
    }

    // dehaze v12 add
    pProcRes->ProcResV12.color_deviate_en =
        pCalibV12->enhance_setting.color_deviate_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    pProcRes->ProcResV12.enh_luma_en =
        pCalibV12->enhance_setting.enh_luma_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;

    // add for rk_aiq_uapi2_setMEnhanceStrth
    if (MEnhanceStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceStrth);
        level_up = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL;
        if (pProcRes->ProcResV12.enh_luma_en) {
            if (level_up) {
                for (int j = 0; j < DHAZ_V12_ENH_LUMA_NUM; j++) {
                    pProcRes->ProcResV12.enh_luma[j] += level_diff * ENH_LUMA_DEFAULT_STEP;
                    pProcRes->ProcResV12.enh_luma[j] =
                        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.enh_luma[j], BIT_10_MAX);
                }
            } else {
                for (int j = 0; j < DHAZ_V12_ENH_LUMA_NUM; j++) {
                    pProcRes->ProcResV12.enh_luma[j] =
                        level_diff * ENH_LUMA_DEFAULT_STEP > pProcRes->ProcResV12.enh_luma[j]
                            ? 0
                            : (pProcRes->ProcResV12.enh_luma[j] -
                               level_diff * ENH_LUMA_DEFAULT_STEP);
                    pProcRes->ProcResV12.enh_luma[j] =
                        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.enh_luma[j], BIT_10_MAX);
                }
            }
        } else {
            if (level_up) {
                pProcRes->ProcResV12.enhance_value += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
            } else {
                pProcRes->ProcResV12.enhance_value =
                    level_diff * ENHANCE_VALUE_DEFAULT_STEP > pProcRes->ProcResV12.enhance_value
                        ? 0
                        : (pProcRes->ProcResV12.enhance_value -
                           level_diff * ENHANCE_VALUE_DEFAULT_STEP);
            }
            pProcRes->ProcResV12.enhance_value =
                LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.enhance_value, BIT_14_MAX);
        }
    }

    // add for rk_aiq_uapi2_setMEnhanceChromeStrth
    if (MEnhanceChromeStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceChromeStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceChromeStrth);
        level_up = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV12.enhance_chroma += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV12.enhance_chroma =
                level_diff * ENHANCE_VALUE_DEFAULT_STEP > pProcRes->ProcResV12.enhance_chroma
                    ? 0
                    : (pProcRes->ProcResV12.enhance_chroma -
                       level_diff * ENHANCE_VALUE_DEFAULT_STEP);
        }
        pProcRes->ProcResV12.enhance_chroma =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV12.enhance_chroma, BIT_14_MAX);
    }

    if (pProcRes->ProcResV12.dc_en && pProcRes->ProcResV12.enhance_en) {
        LOGD_ADEHAZE("%s color_deviate_en:%d enh_luma_en:%d\n", __func__,
                     pProcRes->ProcResV12.color_deviate_en, pProcRes->ProcResV12.enh_luma_en);
        LOGD_ADEHAZE("%s CtrlValue:%f enhance_value:%f enhance_chroma:%f\n", __func__, CtrlValue,
                     pProcRes->ProcResV12.enhance_value / 1024.0f,
                     pProcRes->ProcResV12.enhance_chroma / 1024.0f);
        if (pProcRes->ProcResV12.enh_luma_en) {
            LOGD_ADEHAZE(
                "%s enh_luma[0~7]:%f %f %f %f %f %f %f %f\n", __func__,
                pProcRes->ProcResV12.enh_luma[0] / 64.0f, pProcRes->ProcResV12.enh_luma[1] / 64.0f,
                pProcRes->ProcResV12.enh_luma[2] / 64.0f, pProcRes->ProcResV12.enh_luma[3] / 64.0f,
                pProcRes->ProcResV12.enh_luma[4] / 64.0f, pProcRes->ProcResV12.enh_luma[5] / 64.0f,
                pProcRes->ProcResV12.enh_luma[6] / 64.0f, pProcRes->ProcResV12.enh_luma[7] / 64.0f);
        }
        LOGD_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     pProcRes->ProcResV12.enhance_value, pProcRes->ProcResV12.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetHistParamsV12(CalibDbDehazeV12_t* pCalibV12, RkAiqAdehazeProcResult_t* pProcRes,
                      dehaze_stats_v12_t* pStats, bool stats_true, float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV12.hpara_en =
        pCalibV12->hist_setting.hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    // clip hpara_en
    pProcRes->ProcResV12.hpara_en = pProcRes->ProcResV12.dc_en
                                    ? pProcRes->ProcResV12.hpara_en
                                    : FUNCTION_ENABLE;  //  dc en �رգ�hpara���迪

    pProcRes->ProcResV12.hist_gratio = DehazeLinearInterpV12(
                                           pCalibV12->hist_setting.HistData.CtrlData, pCalibV12->hist_setting.HistData.hist_gratio,
                                           CtrlValue, 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.hist_th_off = DehazeLinearInterpV12(
                                           pCalibV12->hist_setting.HistData.CtrlData, pCalibV12->hist_setting.HistData.hist_th_off,
                                           CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.hist_k   = DehazeLinearInterpV12(pCalibV12->hist_setting.HistData.CtrlData,
                                    pCalibV12->hist_setting.HistData.hist_k,
                                    CtrlValue, 3, 2, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.hist_min = DehazeLinearInterpV12(pCalibV12->hist_setting.HistData.CtrlData,
                                    pCalibV12->hist_setting.HistData.hist_min,
                                    CtrlValue, 1, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.cfg_gratio = DehazeLinearInterpV12(
                                          pCalibV12->hist_setting.HistData.CtrlData, pCalibV12->hist_setting.HistData.cfg_gratio,
                                          CtrlValue, 5, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV12.hist_scale = DehazeLinearInterpV12(
        pCalibV12->hist_setting.HistData.CtrlData, pCalibV12->hist_setting.HistData.hist_scale,
        CtrlValue, 5, 8, DHAZ_CTRL_DATA_STEP_MAX);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pCalibV12->hist_setting.hist_wr.mode == HIST_WR_MANUAL) {
        mManual_curve_t Curve;
        DehazeHistWrTableInterpV12(&pCalibV12->hist_setting.hist_wr, &Curve, CtrlValue);
        ret = TransferHistWr2Res(pProcRes, &Curve);

        if (ret == XCAM_RETURN_NO_ERROR)
            pProcRes->ProcResV12.soft_wr_en = FUNCTION_ENABLE;
        else
            pProcRes->ProcResV12.soft_wr_en = FUNCTION_DISABLE;
    } else if (pCalibV12->hist_setting.hist_wr.mode == HIST_WR_AUTO) {
        pProcRes->ProcResV12.soft_wr_en = FUNCTION_DISABLE;
    } else if (pCalibV12->hist_setting.hist_wr.mode == HIST_WR_SEMIAUTO) {
        mhist_wr_semiauto_t semi_auto_curve;
        semi_auto_curve.clim0 =
            LinearInterpV12(pCalibV12->hist_setting.hist_wr.semiauto_curve.CtrlData,
                            pCalibV12->hist_setting.hist_wr.semiauto_curve.clim0, CtrlValue,
                            DHAZ_CTRL_DATA_STEP_MAX);
        semi_auto_curve.clim1 =
            LinearInterpV12(pCalibV12->hist_setting.hist_wr.semiauto_curve.CtrlData,
                            pCalibV12->hist_setting.hist_wr.semiauto_curve.clim1, CtrlValue,
                            DHAZ_CTRL_DATA_STEP_MAX);
        semi_auto_curve.dark_th =
            LinearInterpV12(pCalibV12->hist_setting.hist_wr.semiauto_curve.CtrlData,
                            pCalibV12->hist_setting.hist_wr.semiauto_curve.dark_th, CtrlValue,
                            DHAZ_CTRL_DATA_STEP_MAX);
        ret = TransferHistWrSemiAtuos2Res(pProcRes, &semi_auto_curve, pStats, stats_true);

        if (ret == XCAM_RETURN_NO_ERROR)
            pProcRes->ProcResV12.soft_wr_en = FUNCTION_ENABLE;
        else
            pProcRes->ProcResV12.soft_wr_en = FUNCTION_DISABLE;
    }

    if (pProcRes->ProcResV12.hist_en) {
        LOGD_ADEHAZE(
            "%s cfg_alpha:%f CtrlValue:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f "
            "hist_min:%f hist_scale:%f cfg_gratio:%f\n",
            __func__, pProcRes->ProcResV12.cfg_alpha / 255.0f, CtrlValue,
            pProcRes->ProcResV12.hpara_en, pProcRes->ProcResV12.hist_gratio / 255.0f,
            pProcRes->ProcResV12.hist_th_off / 1.0f, pProcRes->ProcResV12.hist_k / 4.0f,
            pProcRes->ProcResV12.hist_min / 256.0f, pProcRes->ProcResV12.hist_scale / 256.0f,
            pProcRes->ProcResV12.cfg_gratio / 256.0f);
        LOGD_ADEHAZE(
            "%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x "
            "hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n",
            __func__, pProcRes->ProcResV12.cfg_alpha, pProcRes->ProcResV12.hist_gratio,
            pProcRes->ProcResV12.hist_th_off, pProcRes->ProcResV12.hist_k,
            pProcRes->ProcResV12.hist_min, pProcRes->ProcResV12.hist_scale,
            pProcRes->ProcResV12.cfg_gratio);
        LOGD_ADEHAZE("%s soft_wr_en:%d hist_wr.mode:%d\n", __func__,
                     pProcRes->ProcResV12.soft_wr_en, pCalibV12->hist_setting.hist_wr.mode);
        if (pProcRes->ProcResV12.soft_wr_en) {
            LOGD_ADEHAZE(
                "%s hist_wr[0~9]:0x%x 0x%x 0x%x "
                "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                __func__, pProcRes->ProcResV12.hist_wr[0], pProcRes->ProcResV12.hist_wr[1],
                pProcRes->ProcResV12.hist_wr[2], pProcRes->ProcResV12.hist_wr[3],
                pProcRes->ProcResV12.hist_wr[4], pProcRes->ProcResV12.hist_wr[5],
                pProcRes->ProcResV12.hist_wr[6], pProcRes->ProcResV12.hist_wr[7],
                pProcRes->ProcResV12.hist_wr[8], pProcRes->ProcResV12.hist_wr[9]);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

XCamReturn GetDehazeLocalGainSettingV12(RkAiqAdehazeProcResult_t* pProcRes, float* sigma) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // get sigma_idx
    for (int i = 0; i < DHAZ_V12_SIGMA_IDX_NUM; i++)
        pProcRes->ProcResV12.sigma_idx[i] = (i + 1) * YNR_CURVE_STEP;

    // get sigma_lut
    int tmp = 0;
    for (int i = 0; i < DHAZ_V12_SIGMA_LUT_NUM; i++) {
        tmp                               = LIMIT_VALUE(8.0f * sigma[i], BIT_10_MAX, BIT_MIN);
        pProcRes->ProcResV12.sigma_lut[i] = tmp;
    }

#if 0
    LOGE_ADEHAZE("%s(%d) dehaze stManual sigma_curve(0~4): 0x%f 0x%f 0x%f 0x%f 0x%f\n", __func__, __LINE__, pstManu->sigma_curve[0], pstManu->sigma_curve[1],
                 pstManu->sigma_curve[2], pstManu->sigma_curve[3], pstManu->sigma_curve[4]);
    LOGE_ADEHAZE("%s(%d) dehaze local gain IDX(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, pProcRes->ProcResV12.sigma_idx[0], pProcRes->ProcResV12.sigma_idx[1],
                 pProcRes->ProcResV12.sigma_idx[2], pProcRes->ProcResV12.sigma_idx[3], pProcRes->ProcResV12.sigma_idx[4], pProcRes->ProcResV12.sigma_idx[5]);
    LOGE_ADEHAZE("%s(%d) dehaze local gain LUT(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, pProcRes->ProcResV12.sigma_lut[0], pProcRes->ProcResV12.sigma_lut[1],
                 pProcRes->ProcResV12.sigma_lut[2], pProcRes->ProcResV12.sigma_lut[3], pProcRes->ProcResV12.sigma_lut[4], pProcRes->ProcResV12.sigma_lut[5]);
#endif

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

void AdehazeGetStats(AdehazeHandle_t* pAdehazeCtx, rkisp_adehaze_stats_t* ROData) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);

    pAdehazeCtx->stats.stats_true = ROData->stats_true;
    if (pAdehazeCtx->stats.stats_true) {
        pAdehazeCtx->stats.dehaze_stats_v12.dhaz_adp_air_base =
            ROData->dehaze_stats_v12.dhaz_adp_air_base;
        pAdehazeCtx->stats.dehaze_stats_v12.dhaz_adp_wt = ROData->dehaze_stats_v12.dhaz_adp_wt;
        pAdehazeCtx->stats.dehaze_stats_v12.dhaz_adp_gratio =
            ROData->dehaze_stats_v12.dhaz_adp_gratio;
        pAdehazeCtx->stats.dehaze_stats_v12.dhaz_adp_tmax = ROData->dehaze_stats_v12.dhaz_adp_tmax;
        pAdehazeCtx->stats.dehaze_stats_v12.dhaz_pic_sumh = ROData->dehaze_stats_v12.dhaz_pic_sumh;
        for (int i = 0; i < DHAZ_V12_HIST_WR_NUM; i++)
            pAdehazeCtx->stats.dehaze_stats_v12.h_rgb_iir[i] =
                ROData->dehaze_stats_v12.h_rgb_iir[i];

        LOGV_ADEHAZE(
            "%s:  stats_true:%d dhaz_adp_air_base:%d dhaz_adp_wt:%d dhaz_adp_gratio:%d "
            "dhaz_adp_tmax:%d "
            "dhaz_pic_sumh:%d \n",
            __FUNCTION__, pAdehazeCtx->stats.stats_true,
            pAdehazeCtx->stats.dehaze_stats_v12.dhaz_adp_air_base,
            pAdehazeCtx->stats.dehaze_stats_v12.dhaz_adp_wt,
            pAdehazeCtx->stats.dehaze_stats_v12.dhaz_adp_gratio,
            pAdehazeCtx->stats.dehaze_stats_v12.dhaz_adp_tmax,
            pAdehazeCtx->stats.dehaze_stats_v12.dhaz_pic_sumh);
        LOGV_ADEHAZE(
            "%s h_rgb_iir[0~1]:0x%x 0x%x 0x%x "
            "0x%x 0x%x 0x%x 0x%x\n",
            __func__, pAdehazeCtx->stats.dehaze_stats_v12.h_rgb_iir[0],
            pAdehazeCtx->stats.dehaze_stats_v12.h_rgb_iir[1],
            pAdehazeCtx->stats.dehaze_stats_v12.h_rgb_iir[2],
            pAdehazeCtx->stats.dehaze_stats_v12.h_rgb_iir[3],
            pAdehazeCtx->stats.dehaze_stats_v12.h_rgb_iir[4],
            pAdehazeCtx->stats.dehaze_stats_v12.h_rgb_iir[5],
            pAdehazeCtx->stats.dehaze_stats_v12.h_rgb_iir[6]);
    }

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
}

XCamReturn AdehazeGetCurrDataGroup(AdehazeHandle_t* pAdehazeCtx, RKAiqAecExpInfo_t* pAeEffExpo,
                                   XCamVideoBuffer* pAePreRes) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;

    // get EnvLv
    if (pAePreRes) {
        RkAiqAlgoPreResAe* pAEPreRes = (RkAiqAlgoPreResAe*)pAePreRes->map(pAePreRes);

        switch (pAdehazeCtx->FrameNumber) {
        case LINEAR_NUM:
            pAdehazeCtx->CurrDataV12.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
            break;
        case HDR_2X_NUM:
            pAdehazeCtx->CurrDataV12.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
            break;
        case HDR_3X_NUM:
            pAdehazeCtx->CurrDataV12.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
            break;
        default:
            LOGE_ADEHAZE("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
            break;
        }

        // Normalize the current envLv for AEC
        pAdehazeCtx->CurrDataV12.EnvLv =
            (pAdehazeCtx->CurrDataV12.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
        pAdehazeCtx->CurrDataV12.EnvLv =
            LIMIT_VALUE(pAdehazeCtx->CurrDataV12.EnvLv, ENVLVMAX, ENVLVMIN);
    } else {
        pAdehazeCtx->CurrDataV12.EnvLv = ENVLVMIN;
        LOGW_ADEHAZE("%s:PreResBuf is NULL!\n", __FUNCTION__);
    }

    // get iso
    if (pAeEffExpo) {
        if (pAdehazeCtx->FrameNumber == LINEAR_NUM) {
            pAdehazeCtx->CurrDataV12.ISO = pAeEffExpo->LinearExp.exp_real_params.analog_gain *
                                           pAeEffExpo->LinearExp.exp_real_params.digital_gain *
                                           pAeEffExpo->LinearExp.exp_real_params.isp_dgain * ISOMIN;
            // ablcV32_proc_res not ready for now
            pAdehazeCtx->ablcV32_proc_res.blc_ob_enable   = false;
            pAdehazeCtx->ablcV32_proc_res.isp_ob_predgain = 1.0f;
            if (pAdehazeCtx->ablcV32_proc_res.blc_ob_enable)
                pAdehazeCtx->CurrDataV12.ISO *= pAdehazeCtx->ablcV32_proc_res.isp_ob_predgain;
        } else if (pAdehazeCtx->FrameNumber == HDR_2X_NUM ||
                   pAdehazeCtx->FrameNumber == HDR_3X_NUM) {
            pAdehazeCtx->CurrDataV12.ISO = pAeEffExpo->HdrExp[1].exp_real_params.analog_gain *
                                           pAeEffExpo->HdrExp[1].exp_real_params.digital_gain *
                                           pAeEffExpo->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    } else {
        pAdehazeCtx->CurrDataV12.ISO = ISOMIN;
        LOGW_ADEHAZE("%s:AE cur expo is NULL!\n", __FUNCTION__);
    }

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
    return ret;
}

XCamReturn AdehazeGetCurrData(AdehazeHandle_t* pAdehazeCtx, RkAiqAlgoProcAdhaz* pProcPara) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;

    // get ynr res
    for (int i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++)
        pAdehazeCtx->YnrProcResV22_sigma[i] = pProcPara->aynrV22_proc_res.stSelect.sigma[i];

    // get EnvLv
    XCamVideoBuffer* xCamAePreRes = pProcPara->com.u.proc.res_comb->ae_pre_res;
    if (xCamAePreRes) {
        RkAiqAlgoPreResAe* pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);

        switch (pAdehazeCtx->FrameNumber) {
        case LINEAR_NUM:
            pAdehazeCtx->CurrDataV12.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
            break;
        case HDR_2X_NUM:
            pAdehazeCtx->CurrDataV12.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
            break;
        case HDR_3X_NUM:
            pAdehazeCtx->CurrDataV12.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
            break;
        default:
            LOGE_ADEHAZE("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
            break;
        }
        // Normalize the current envLv for AEC
        pAdehazeCtx->CurrDataV12.EnvLv =
            (pAdehazeCtx->CurrDataV12.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
        pAdehazeCtx->CurrDataV12.EnvLv =
            LIMIT_VALUE(pAdehazeCtx->CurrDataV12.EnvLv, ENVLVMAX, ENVLVMIN);
    } else {
        pAdehazeCtx->CurrDataV12.EnvLv = ENVLVMIN;
        LOGW_ADEHAZE("%s:PreResBuf is NULL!\n", __FUNCTION__);
    }

    // get ISO
    if (pProcPara->com.u.proc.curExp) {
        if (pAdehazeCtx->FrameNumber == LINEAR_NUM) {
            pAdehazeCtx->CurrDataV12.ISO =
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;
        } else if (pAdehazeCtx->FrameNumber == HDR_2X_NUM ||
                   pAdehazeCtx->FrameNumber == HDR_3X_NUM) {
            pAdehazeCtx->CurrDataV12.ISO =
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    } else {
        pAdehazeCtx->CurrDataV12.ISO   = ISOMIN;
        LOGW_ADEHAZE("%s:AE cur expo is NULL!\n", __FUNCTION__);
    }

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
    return ret;
}

XCamReturn AdehazeInit(AdehazeHandle_t** pAdehazeCtx, CamCalibDbV2Context_t* pCalib) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret          = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* handle = (AdehazeHandle_t*)calloc(1, sizeof(AdehazeHandle_t));

    CalibDbV2_dehaze_v12_t* calibv2_adehaze_calib_V12 =
        (CalibDbV2_dehaze_v12_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, adehaze_calib));
    memcpy(&handle->AdehazeAtrrV12.stAuto, calibv2_adehaze_calib_V12,
           sizeof(CalibDbV2_dehaze_v12_t));  // set defsult stAuto

    handle->PreDataV12.EnvLv   = ENVLVMIN;
    handle->PreDataV12.ApiMode = DEHAZE_API_AUTO;

    // set api default
    handle->AdehazeAtrrV12.mode                                               = DEHAZE_API_AUTO;

    handle->AdehazeAtrrV12.Info.ISO                 = ISOMIN;
    handle->AdehazeAtrrV12.Info.EnvLv               = ENVLVMIN;
    handle->AdehazeAtrrV12.Info.MDehazeStrth        = DEHAZE_DEFAULT_LEVEL;
    handle->AdehazeAtrrV12.Info.MEnhanceStrth       = ENHANCE_DEFAULT_LEVEL;
    handle->AdehazeAtrrV12.Info.MEnhanceChromeStrth = ENHANCE_DEFAULT_LEVEL;

    *pAdehazeCtx = handle;
    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return (ret);
}

XCamReturn AdehazeRelease(AdehazeHandle_t* pAdehazeCtx) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pAdehazeCtx) free(pAdehazeCtx);
    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return (ret);
}

XCamReturn AdehazeProcess(AdehazeHandle_t* pAdehazeCtx) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Dehaze module en:%d Dehaze en:%d, Enhance en:%d, Hist en:%d\n", __func__,
                 pAdehazeCtx->ProcRes.enable,
                 (pAdehazeCtx->ProcRes.ProcResV12.dc_en & FUNCTION_ENABLE) &&
                     (!(pAdehazeCtx->ProcRes.ProcResV12.enhance_en & FUNCTION_ENABLE)),
                 (pAdehazeCtx->ProcRes.ProcResV12.dc_en & FUNCTION_ENABLE) &&
                     (pAdehazeCtx->ProcRes.ProcResV12.enhance_en & FUNCTION_ENABLE),
                 pAdehazeCtx->ProcRes.ProcResV12.hist_en);

    if (pAdehazeCtx->AdehazeAtrrV12.mode == DEHAZE_API_AUTO) {
        // cfg setting
        pAdehazeCtx->ProcRes.ProcResV12.cfg_alpha =
            LIMIT_VALUE(SHIFT8BIT(pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.cfg_alpha),
                        BIT_8_MAX, BIT_MIN);

        float CtrlValue = pAdehazeCtx->CurrDataV12.EnvLv;
        if (pAdehazeCtx->CurrDataV12.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAdehazeCtx->CurrDataV12.ISO;

        // dehaze setting
        if (pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.dehaze_setting.en ||
            pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.enhance_setting.en ||
            (pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.hist_setting.en &&
             !pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.hist_setting.hist_para_en))
            GetDehazeParamsV12(&pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara,
                               &pAdehazeCtx->ProcRes, pAdehazeCtx->width, pAdehazeCtx->height,
                               pAdehazeCtx->AdehazeAtrrV12.Info.MDehazeStrth, CtrlValue);

        // fix register
        pAdehazeCtx->ProcRes.ProcResV12.round_en = FUNCTION_ENABLE;

        // enhance setting
        if (pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.enhance_setting.en)
            GetEnhanceParamsV12(&pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara,
                                &pAdehazeCtx->ProcRes,
                                pAdehazeCtx->AdehazeAtrrV12.Info.MEnhanceStrth,
                                pAdehazeCtx->AdehazeAtrrV12.Info.MEnhanceChromeStrth, CtrlValue);

        // hist setting
        if (pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.hist_setting.en)
            GetHistParamsV12(&pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara,
                             &pAdehazeCtx->ProcRes, &pAdehazeCtx->stats.dehaze_stats_v12,
                             pAdehazeCtx->stats.stats_true, CtrlValue);
    } else if (pAdehazeCtx->AdehazeAtrrV12.mode == DEHAZE_API_MANUAL) {
        // cfg setting
        pAdehazeCtx->ProcRes.ProcResV12.cfg_alpha = LIMIT_VALUE(
            SHIFT8BIT(pAdehazeCtx->AdehazeAtrrV12.stManual.cfg_alpha), BIT_8_MAX, BIT_MIN);

        // fix register
        pAdehazeCtx->ProcRes.ProcResV12.round_en = FUNCTION_ENABLE;

        // dehaze setting
        if (pAdehazeCtx->AdehazeAtrrV12.stManual.dehaze_setting.en ||
            pAdehazeCtx->AdehazeAtrrV12.stManual.enhance_setting.en ||
            (pAdehazeCtx->AdehazeAtrrV12.stManual.hist_setting.en &&
             !pAdehazeCtx->AdehazeAtrrV12.stManual.hist_setting.hist_para_en))
            stManuGetDehazeParamsV12(&pAdehazeCtx->AdehazeAtrrV12.stManual, &pAdehazeCtx->ProcRes,
                                     pAdehazeCtx->width, pAdehazeCtx->height,
                                     pAdehazeCtx->AdehazeAtrrV12.Info.MDehazeStrth);

        // enhance setting
        if (pAdehazeCtx->AdehazeAtrrV12.stManual.enhance_setting.en)
            stManuGetEnhanceParamsV12(&pAdehazeCtx->AdehazeAtrrV12.stManual, &pAdehazeCtx->ProcRes,
                                      pAdehazeCtx->AdehazeAtrrV12.Info.MEnhanceStrth,
                                      pAdehazeCtx->AdehazeAtrrV12.Info.MEnhanceChromeStrth);

        // hist setting
        if (pAdehazeCtx->AdehazeAtrrV12.stManual.hist_setting.en)
            stManuGetHistParamsV12(&pAdehazeCtx->AdehazeAtrrV12.stManual, &pAdehazeCtx->ProcRes,
                                   &pAdehazeCtx->stats.dehaze_stats_v12,
                                   pAdehazeCtx->stats.stats_true);
    } else
        LOGE_ADEHAZE("%s:Wrong Adehaze API mode!!! \n", __func__);

    // get local gain setting
    ret = GetDehazeLocalGainSettingV12(&pAdehazeCtx->ProcRes, pAdehazeCtx->YnrProcResV22_sigma);

    // store pre data
    pAdehazeCtx->PreDataV12.EnvLv = pAdehazeCtx->CurrDataV12.EnvLv;
    pAdehazeCtx->PreDataV12.ISO     = pAdehazeCtx->CurrDataV12.ISO;
    pAdehazeCtx->PreDataV12.ApiMode = pAdehazeCtx->CurrDataV12.ApiMode;

    // store api info
    pAdehazeCtx->AdehazeAtrrV12.Info.ISO   = pAdehazeCtx->CurrDataV12.ISO;
    pAdehazeCtx->AdehazeAtrrV12.Info.EnvLv = pAdehazeCtx->CurrDataV12.EnvLv;

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return ret;
}

bool AdehazeByPassProcessing(AdehazeHandle_t* pAdehazeCtx) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pAdehazeCtx->CurrDataV12.CtrlDataType =
        pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.CtrlDataType;

    if (pAdehazeCtx->FrameID <= 4)
        pAdehazeCtx->byPassProc = false;
    else if (pAdehazeCtx->AdehazeAtrrV12.mode != pAdehazeCtx->PreDataV12.ApiMode)
        pAdehazeCtx->byPassProc = false;
    else if (pAdehazeCtx->AdehazeAtrrV12.mode == DEHAZE_API_MANUAL) {
        pAdehazeCtx->byPassProc = !pAdehazeCtx->ifReCalcStManual;
    } else if (pAdehazeCtx->AdehazeAtrrV12.mode == DEHAZE_API_AUTO) {
        float diff = 0.0;
        if (pAdehazeCtx->CurrDataV12.CtrlDataType == CTRLDATATYPE_ENVLV) {
            diff = pAdehazeCtx->PreDataV12.EnvLv - pAdehazeCtx->CurrDataV12.EnvLv;
            if (pAdehazeCtx->PreDataV12.EnvLv == ENVLVMIN) {
                diff = pAdehazeCtx->CurrDataV12.EnvLv;
                if (diff == 0.0)
                    pAdehazeCtx->byPassProc = true;
                else
                    pAdehazeCtx->byPassProc = false;
            } else {
                diff /= pAdehazeCtx->PreDataV12.EnvLv;
                if (diff >= pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.ByPassThr ||
                    diff <= (0 - pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.ByPassThr))
                    pAdehazeCtx->byPassProc = false;
                else
                    pAdehazeCtx->byPassProc = true;
            }
        } else if (pAdehazeCtx->CurrDataV12.CtrlDataType == CTRLDATATYPE_ISO) {
            diff = pAdehazeCtx->PreDataV12.ISO - pAdehazeCtx->CurrDataV12.ISO;
            diff /= pAdehazeCtx->PreDataV12.ISO;
            if (diff >= pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.ByPassThr ||
                diff <= (0 - pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.ByPassThr))
                pAdehazeCtx->byPassProc = false;
            else
                pAdehazeCtx->byPassProc = true;
        }
        pAdehazeCtx->byPassProc = pAdehazeCtx->byPassProc && !pAdehazeCtx->ifReCalcStAuto;
    }

    LOGD_ADEHAZE(
        "%s:FrameID:%d DehazeApiMode:%d ifReCalcStAuto:%d ifReCalcStManual:%d CtrlDataType:%d "
        "EnvLv:%f ISO:%f byPassProc:%d\n",
        __func__, pAdehazeCtx->FrameID, pAdehazeCtx->AdehazeAtrrV12.mode,
        pAdehazeCtx->ifReCalcStAuto, pAdehazeCtx->ifReCalcStManual,
        pAdehazeCtx->CurrDataV12.CtrlDataType, pAdehazeCtx->CurrDataV12.EnvLv,
        pAdehazeCtx->CurrDataV12.ISO, pAdehazeCtx->byPassProc);

    pAdehazeCtx->ifReCalcStManual = false;
    pAdehazeCtx->ifReCalcStAuto   = false;

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return pAdehazeCtx->byPassProc;
}

/******************************************************************************
 * DehazeEnableSetting()
 *
 *****************************************************************************/
bool DehazeEnableSetting(AdehazeHandle_t* pAdehazeCtx) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);

    if (pAdehazeCtx->AdehazeAtrrV12.mode == DEHAZE_API_AUTO) {
        pAdehazeCtx->ProcRes.enable = pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.Enable;

        if (pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.Enable) {
            if (pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.dehaze_setting.en &&
                pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV12.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV12.enhance_en = FUNCTION_ENABLE;
            } else if (pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.dehaze_setting.en &&
                       !pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV12.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV12.enhance_en = FUNCTION_DISABLE;
            } else if (!pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.dehaze_setting.en &&
                       pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV12.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV12.enhance_en = FUNCTION_ENABLE;
            } else {
                pAdehazeCtx->ProcRes.ProcResV12.dc_en      = FUNCTION_DISABLE;
                pAdehazeCtx->ProcRes.ProcResV12.enhance_en = FUNCTION_DISABLE;
            }

            if (pAdehazeCtx->AdehazeAtrrV12.stAuto.DehazeTuningPara.hist_setting.en)
                pAdehazeCtx->ProcRes.ProcResV12.hist_en = FUNCTION_ENABLE;
            else
                pAdehazeCtx->ProcRes.ProcResV12.hist_en = FUNCTION_DISABLE;
        }
    } else if (pAdehazeCtx->AdehazeAtrrV12.mode == DEHAZE_API_MANUAL) {
        pAdehazeCtx->ProcRes.enable = pAdehazeCtx->AdehazeAtrrV12.stManual.Enable;

        if (pAdehazeCtx->AdehazeAtrrV12.stManual.Enable) {
            if (pAdehazeCtx->AdehazeAtrrV12.stManual.dehaze_setting.en &&
                pAdehazeCtx->AdehazeAtrrV12.stManual.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV12.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV12.enhance_en = FUNCTION_ENABLE;
            } else if (pAdehazeCtx->AdehazeAtrrV12.stManual.dehaze_setting.en &&
                       !pAdehazeCtx->AdehazeAtrrV12.stManual.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV12.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV12.enhance_en = FUNCTION_DISABLE;
            } else if (!pAdehazeCtx->AdehazeAtrrV12.stManual.dehaze_setting.en &&
                       pAdehazeCtx->AdehazeAtrrV12.stManual.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV12.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV12.enhance_en = FUNCTION_ENABLE;
            } else {
                pAdehazeCtx->ProcRes.ProcResV12.dc_en      = FUNCTION_DISABLE;
                pAdehazeCtx->ProcRes.ProcResV12.enhance_en = FUNCTION_DISABLE;
            }

            if (pAdehazeCtx->AdehazeAtrrV12.stManual.hist_setting.en)
                pAdehazeCtx->ProcRes.ProcResV12.hist_en = FUNCTION_ENABLE;
            else
                pAdehazeCtx->ProcRes.ProcResV12.hist_en = FUNCTION_DISABLE;
        }
    } else {
        LOGE_ADEHAZE("%s: Dehaze api in WRONG MODE!!!, dehaze by pass!!!\n", __FUNCTION__);
        pAdehazeCtx->ProcRes.enable = false;
    }

    return pAdehazeCtx->ProcRes.enable;
    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
}
