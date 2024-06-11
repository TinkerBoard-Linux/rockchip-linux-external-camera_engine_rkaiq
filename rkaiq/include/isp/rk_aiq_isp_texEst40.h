/*
 *  Copyright (c) 2023 Rockchip Corporation
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

#ifndef _RK_AIQ_PARAM_TEXEST40_H_
#define _RK_AIQ_PARAM_TEXEST40_H_

typedef enum texEst_filtCfg_mode_e {
    // @note: The filter coefficients for configuring filters are generated by inputting  filter strength into the formula
    // @para: kernel_sigma_enable == 1
    texEst_cfgByFiltStrg_mode = 0,
    // @note: The filter coefficients for configuring filters are directly input.
    // @para: kernel_sigma_enable == 0
    texEst_cfgByFiltCoeff_mode = 1
} texEst_filtCfg_mode_t;

typedef enum texEst_noiseEstThd_mode_e {
    // @note: hw_sharp_noiseCurve_mode == 0
    texEst_baseNoiseStats_mode = 0,
    // @note: hw_sharp_noiseCurve_mode == 1
    texEst_baseManualCfg_mode = 1
} texEst_noiseEstThd_mode_t;

typedef enum texEst_noiseEst_mode_e {
    // @note: hw_sharp_noiseCalc_mode == 0
    texEst_noiseEst3x3_mode = 0,
    // @note: hw_sharp_noiseCalc_mode == 1
    texEst_noiseEst5x5_mode = 1
} texEst_noiseEst_mode_t;

typedef enum texEst_texEst_mode_e {
    // @note: sw_sharp_noiseFilt_sel == 0
    texEst_texEst5x5_mode = 0,
    // @note: sw_sharp_noiseFilt_sel == 1
    texEst_texEst7x7_mode = 1
} texEst_texEst_mode_t;

typedef enum texEst_texEstWgtOpt_mode_e {
    // @note: hw_sharp_texWgt_mode == 0
    texEst_texEstDf12Wgt_mode = 0,
    // @note: hw_sharp_texWgt_mode == 1
    texEst_texEstDf1Only_mode = 1,
	// @note: hw_sharp_texWgt_mode == 2
    texEst_texEstDf2Only_mode = 2,
	// @note: hw_sharp_texWgt_mode == 3
    texEst_texEstFixMax_mode = 3
} texEst_texEstWgtOpt_mode_t;

typedef struct texEst_noiseEst_dyn_s{
	/* M4_GENERIC_DESC(
        M4_ALIAS(noiseCurve_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(texEst_noiseEstThd_mode_t),
        M4_DEFAULT(texEst_baseNoiseStats_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
		M4_GROUP_CTRL(noiseCurve_mode_group),
        M4_NOTES(The mode of the noise curve. \n
        Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_sharp_noiseCurve_mode
    texEst_noiseEstThd_mode_t hw_texEstT_nsEstThd_mode;
	/* M4_GENERIC_DESC(
        M4_ALIAS(noise_curve_ext),
        M4_TYPE(u16),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,7),
        M4_DEFAULT([7, 22, 57, 86, 98, 103, 108, 123, 134, 134, 132, 119, 125, 182, 307, 277, 277]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_RO(0),
        M4_ORDER(0),
		M4_GROUP(noiseCurve_mode_group:texEst_noiseManualCurve_mode),
        M4_NOTES(The manual noise curve value when noiseCurve_mode  == texEst_noiseManualCurve_mode. \n
        Freq of use: high))  */
    // @reg: hw_sharp_noise_curve_ext0~16
    uint16_t hw_texEstT_nsEstManual_thred[17];
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseCountThred_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0.249),
        M4_DEFAULT(0.003),
        M4_DIGIT_EX(4),
        M4_FP_EX(0,4,10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
		M4_GROUP(noiseCurve_mode_group:texEst_noiseAutoCount_mode),
        M4_NOTES(The lower limit of the count of pixel when noise estimation.\n
        Freq of use: high))  */
    // @reg: hw_texEst_noiseCountThred_ratio
    float hw_texEstT_nsStatsCntThd_ratio;
	/* M4_GENERIC_DESC(
        M4_ALIAS(noiseCalc_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(texEst_noiseEst_mode_t),
        M4_DEFAULT(texEst_noiseEst5x5_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
		M4_GROUP(noiseCurve_mode_group:texEst_noiseAutoCount_mode),
        M4_NOTES(The mode of the noise curve. \n
        Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_sharp_noiseCalc_mode
    texEst_noiseEst_mode_t hw_texEstT_noiseEst_mode;	
	/* M4_GENERIC_DESC(
        M4_ALIAS(noiseEst_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
		M4_GROUP(noiseCurve_mode_group:texEst_noiseAutoCount_mode),
        M4_NOTES(The weight of noise est mean value between noise est min value .\n
		The higer value means the bigger weight of noise est mean value.\n
        Freq of use: low))  */
    // @reg: hw_sharp_noiseEst_alpha
    float hw_texEstT_nsEstMean_alpha;	
	/* M4_GENERIC_DESC(
        M4_ALIAS(noiseClip_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,4,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
		M4_GROUP(noiseCurve_mode_group:texEst_noiseAutoCount_mode),
        M4_NOTES(The scaling ratio of noise statistics in the previous frame, used as a texture threshold.\n
        Lower the value, the lower the noise into noise statistics .\n
        Freq of use: low))  */
    // @reg: hw_sharp_noiseClip_scale
    float hw_texEstT_nsEstThd_scale;
	/* M4_GENERIC_DESC(
        M4_ALIAS(noiseClip_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2047.0),
        M4_DEFAULT(20.0),
        M4_DIGIT_EX(1),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
		M4_GROUP(noiseCurve_mode_group:texEst_noiseAutoCount_mode),
        M4_NOTES(the lower limit of noise estimation threshold.\n
        Freq of use: low))  */
    // @reg: hw_sharp_noiseClip_minLimit
    float hw_texEstT_nsEstThd_minLimit;
	/* M4_GENERIC_DESC(
        M4_ALIAS(noiseClip_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2047.0),
        M4_DEFAULT(1023.0),
        M4_DIGIT_EX(1),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
		M4_GROUP(noiseCurve_mode_group:texEst_noiseAutoCount_mode),
        M4_NOTES(the higer limit of noise estimation threshold.\n
        Freq of use: low))  */
    // @reg: hw_sharp_noiseClip_maxLimit
    float hw_texEstT_nsEstThd_maxLimit;
}texEst_noiseEst_dyn_t;

typedef struct texEst_texEst_dyn_s{
    /* M4_GENERIC_DESC(
        M4_ALIAS(texEst_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(texEst_texEst_mode_t),
        M4_DEFAULT(texEst_texEst7x7_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
		M4_GROUP(texWgt_mode_group:sharp_fstWgt_x_secWgt_mode|sharp_fstWgtOnly_mode|sharp_secWgtOnly_mode),
        M4_NOTES(SHARP derivative estiamtion mode. \n
        Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_sharp_texEst_mode
    texEst_texEst_mode_t hw_texEstT_texEst_mode;
	/* M4_GENERIC_DESC(
        M4_ALIAS(fstNoise_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,10,6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The noise scale value when calculating the first-order derivative weight.\n
        Freq of use: low))  */
    // @reg: hw_sharp_fstNoise_scale
    float hw_texEstT_nsEstDf1_scale;	
	/* M4_GENERIC_DESC(
        M4_ALIAS(fstSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,10,6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The noise scale of denominator  when calculating first-order derivative weight.\n
        Freq of use: low))  */
    // @reg: hw_sharp_fstSigma_scale
    float hw_texEstT_sigmaDf1_scale;
	/* M4_GENERIC_DESC(
        M4_ALIAS(fstSigma_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,16,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The noise offset of denominator when calculating first-order derivative weight.\n
        Freq of use: low))  */
    // @reg: hw_sharp_fstSigma_offset
    float hw_texEstT_sigmaDf1_offset;
	/* M4_GENERIC_DESC(
        M4_ALIAS(fstWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32.0),
        M4_DEFAULT(6.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,5,10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(the scaling coefficient of first-order derivative weight.\n
        Freq of use: low))  */
    // @reg: hw_sharp_fstWgt_scale
    float hw_texEstT_texEstDf1_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(secNoise_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,10,6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The noise scale value when calculating the first-order derivative weight.\n
        Freq of use: low))  */
    // @reg: hw_sharp_secNoise_scale
    float hw_texEstT_nsEstDf2_scale;	
	/* M4_GENERIC_DESC(
        M4_ALIAS(secSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,10,6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The noise scale of denominator  when calculating first-order derivative weight.\n
        Freq of use: low))  */
    // @reg: hw_sharp_secSigma_scale
    float hw_texEstT_sigmaDf2_scale;
	/* M4_GENERIC_DESC(
        M4_ALIAS(secSigma_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,16,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The noise offset of denominator when calculating first-order derivative weight.\n
        Freq of use: low))  */
    // @reg: hw_sharp_secSigma_offset
    float hw_texEstT_sigmaDf2_offset;
	/* M4_GENERIC_DESC(
        M4_ALIAS(secWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32.0),
        M4_DEFAULT(6.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,5,10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(the scaling coefficient of first-order derivative weight.\n
        Freq of use: low))  */
    // @reg: hw_sharp_secWgt_scale
    float hw_texEstT_texEstDf2_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(texWgt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(texEst_texEstWgtOpt_mode_t),
        M4_DEFAULT(texEst_texEstDf12Wgt_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
		M4_GROUP(texWgt_mode_group:sharp_fstWgt_x_secWgt_mode|sharp_fstWgtOnly_mode|sharp_secWgtOnly_mode),
        M4_NOTES(SHARP derivative estiamtion mode. \n
        Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_sharp_texWgt_mode
    texEst_texEstWgtOpt_mode_t hw_texEstT_wgtOpt_mode;
}texEst_texEst_dyn_t;

typedef struct texEst_texEstFlt_s{
		/* M4_GENERIC_DESC(
        M4_ALIAS(texWgtFltCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(texEst_filtCfg_mode_t),
        M4_DEFAULT(texEst_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
		M4_GROUP_CTRL(texWgtFltCfg_mode_group),
        M4_NOTES(texEst texture weight output mode. \n
        Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_sharp_texWgtFlt_coeff0~2
    texEst_filtCfg_mode_t sw_texEstT_filtCfg_mode;
	/* M4_GENERIC_DESC(
        M4_ALIAS(detailBifilt_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,0,6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
		M4_GROUP(texWgtFltCfg_mode_group:texEst_cfgByFiltStrg_mode),
        M4_NOTES(The strength of 3*3 texture weight filter.\n
        Freq of use: low))  */
    // @reg: hw_sharp_texWgtFlt_coeff0~2
    float sw_texEstT_filtSpatial_strg;
	 /* M4_GENERIC_DESC(
        M4_ALIAS(detailBifilt_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT([0.2042,0.1238,0.0751]),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
		M4_GROUP(texWgtFltCfg_mode_group:texEst_cfgByFiltCoeff_mode),
        M4_NOTES(3*3 texture weight filter coefficient.\n
       hw_sharp_texWgtFlt_coeff0 + 4*hw_sharp_texWgtFlt_coeff1 + 4*hw_sharp_texWgtFlt_coeff2 must be equal to 64.\n
        Freq of use: low))  */
    // @reg: hw_sharp_texWgtFlt_coeff0~2
    float sw_texEstT_filtSpatial_wgt[3];
}texEst_texEstFlt_t;

typedef struct texEst_dyn_s {
	/* M4_GENERIC_DESC(
        M4_ALIAS(noiseEst),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
		M4_GROUP(texWgt_mode_group:sharp_fstWgt_x_secWgt_mode|sharp_fstWgtOnly_mode|sharp_secWgtOnly_mode),
        M4_NOTES(TODO))  */
    texEst_noiseEst_dyn_t noiseEst;    	
	/* M4_GENERIC_DESC(
        M4_ALIAS(texFst),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
		M4_GROUP(texWgt_mode_group:sharp_fstWgt_x_secWgt_mode|sharp_fstWgtOnly_mode),
        M4_NOTES(TODO))  */
    texEst_texEst_dyn_t texEst;
	/* M4_GENERIC_DESC(
        M4_ALIAS(texWgtFilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
		M4_GROUP(texWgt_mode_group:sharp_fstWgt_x_secWgt_mode|sharp_fstWgtOnly_mode|sharp_secWgtOnly_mode),
        M4_NOTES(TODO))  */
    texEst_texEstFlt_t texEstFlt;	
} texEst_params_dyn_t;

typedef struct texEst_param_s {
	/* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    texEst_params_dyn_t dyn;
} texEst_param_t;

#endif

