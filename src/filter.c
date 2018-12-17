/*
 * This file is part of mod-host.
 *
 * mod-host is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mod-host is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mod-host.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
************************************************************************************************************************
*
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <math.h>
#include "filter.h"

/*
************************************************************************************************************************
*           LOCAL DEFINES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL CONSTANTS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL DATA TYPES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL MACROS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL GLOBAL VARIABLES
************************************************************************************************************************
*/

const unsigned int filter_order = 512;
static double delta_t[512] = { 0.0 }; // all elements 0.0

/*
************************************************************************************************************************
*           LOCAL FUNCTION PROTOTYPES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL CONFIGURATION ERRORS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL FUNCTIONS
************************************************************************************************************************
*/

/**
 * Calculate the BPM from the time difference of two adjacent MIDI
 * Beat Clock signals.
 * 
 * `delta_t` is time in samples. Due to filtering, this is not integer.
 */
float beats_per_minute(const float delta_t, const jack_nframes_t sample_rate) {
  /*
   * \text{bpm} = \frac{120}{2\cdot{}24}\cdot{}\cfrac{\text{SR}}{\delta t}
   */  
  return (2.5 * (sample_rate)) / delta_t;
}

/**
 * `raw_delta_t` is the time difference in samples between two
 * adjacent MIDI Beat Clock ticks. Over time this has jitter. This
 * function filters the jitter and returns a more steady time delta.
 *
 * Currently this filter is implemented as a centered binomial FIR
 * filter.
 */
float beat_clock_tick_filter(unsigned long long raw_delta_t) {
  float result = 0.0;

  /* These coefficients were calculated using the following Python
   * script:
   *
   * # Pascal's triangle as binomial coefficients, but normalized such that the sum is 1.
   * # Print the k-th row of the triangle:
   * def normalized_binomial_coefficients(k):
   *     coeffs = []
   *     for i in range(k-1, k):
   *         for n in range(0, i+1):
   *             coeffs.append(scipy.special.comb(i, n, exact=True))
   *     s = sum(coeffs)
   *     print([e/s for e in coeffs])
   *
   * normalized_binomial_coefficients(512)
   */  
  const double coeffs[] = {
1.4916681462400413e-154, 7.622424227286611e-152, 1.9437181779580859e-149, 3.2978418419355524e-147, 4.1882591392581515e-145, 4.246894767207766e-143, 3.5815479203452157e-141, 2.58383099967762e-139, 1.6278135297969005e-137, 9.0976689498649e-136, 4.56702981283218e-134, 2.080074487480838e-132, 8.666977031170159e-131, 3.3267857988876226e-129, 1.18338523417574e-127, 3.920949742568952e-126, 1.2154944201963751e-124, 3.539233752924739e-123, 9.713230410804562e-122, 2.520327680277184e-120, 6.200006093481872e-119, 1.4496204723331426e-117, 3.2287001429238176e-116, 6.864497260390203e-115, 1.3957811096126747e-113, 2.71898160152549e-112, 5.082404070543801e-111, 9.12950360819905e-110, 1.5780999094172644e-108, 2.6283526077535817e-107, 4.222886523124088e-106, 6.552285218137697e-105, 9.828427827206546e-104, 1.426611190676344e-102, 2.005647497480272e-101, 2.7334110179945424e-100, 3.61417679045945e-99, 4.6398215553195645e-98, 5.78756688742493e-97, 7.019279840389723e-96, 8.282750211659874e-95, 9.515061828516586e-94, 1.0647807284292371e-92, 1.1613538642635167e-91, 1.2352582010802858e-90, 1.2819235108988745e-89, 1.2986442523453815e-88, 1.2848288879587286e-87, 1.2420012583601042e-86, 1.1735644543280168e-85, 1.0843735557990875e-84, 9.80188645536038e-84, 8.670899556664952e-83, 7.509326219828704e-82, 6.369021127188049e-81, 5.2920775547726144e-80, 4.309263151743415e-79, 3.4398504106021997e-78, 2.692572562781722e-77, 2.0673480863391865e-76, 1.557402225042187e-75, 1.151456399170535e-74, 8.357344832689367e-74, 5.956266396630994e-73, 4.1693864776416954e-72, 2.8672550084705196e-71, 1.937569293602806e-70, 1.2868930382884307e-69, 8.402654544118577e-69, 5.394747772528304e-68, 3.406397879225015e-67, 2.115804879913002e-66, 1.2929918710579457e-65, 7.775663443759427e-65, 4.6023521464413906e-64, 2.6816371839931834e-63, 1.538412910817142e-62, 8.691033976694244e-62, 4.835780443442695e-61, 2.6504973822920087e-60, 1.4312685864376847e-59, 7.615762478452372e-59, 3.993631543578683e-58, 2.064178231560548e-57, 1.0517479560808506e-56, 5.283486791135567e-56, 2.6171690383997113e-55, 1.2785021164596291e-54, 6.160055652032759e-54, 2.9277567874268057e-53, 1.3727926269934576e-52, 6.3510516040027e-52, 2.89939312356645e-51, 1.3062857191121962e-50, 5.808802453073383e-50, 2.549758550454317e-49, 1.1048953718635373e-48, 4.727129683746061e-48, 1.9969711112968055e-47, 8.3307986764200065e-47, 3.432289054685043e-46, 1.3967037638371808e-45, 5.614201403659257e-45, 2.229328518540423e-44, 8.745827265043198e-44, 3.3900492351167446e-43, 1.2984528202428286e-42, 4.9147045999845377e-42, 1.8384635725868085e-41, 6.797255227087009e-41, 2.4840878193536156e-40, 8.974046987034233e-40, 3.205016781083655e-39, 1.1316829165065295e-38, 3.9509631646456024e-38, 1.3639411968385255e-37, 4.656213051276345e-37, 1.571969363465091e-36, 5.2487790610614054e-36, 1.733420311762296e-35, 5.662506351756834e-35, 1.8297851103610926e-34, 5.84931305771169e-34, 1.8499046987397132e-33, 5.7884114767016835e-33, 1.7920921931868412e-32, 5.490060210873974e-32, 1.6643095914854172e-31, 4.992928774456252e-31, 1.482396682648639e-30, 4.355965636706001e-30, 1.26688771571373e-29, 3.64710099978195e-29, 1.0392866758777137e-28, 2.931719130461013e-28, 8.18709712728742e-28, 2.26349155872064e-27, 6.195688573140438e-27, 1.679121395909075e-26, 4.505843745856727e-26, 1.1972670524705017e-25, 3.1502558614649374e-25, 8.2084131601551185e-25, 2.1181150042638032e-24, 5.4129605664519416e-24, 1.37003898475025e-23, 3.434481290538298e-23, 8.527793680588291e-23, 2.0973762836041472e-22, 5.109715375491983e-22, 1.2331446439520652e-21, 2.948114016335732e-21, 6.982375301847786e-21, 1.638348191740755e-20, 3.808627614566171e-20, 8.772129409033052e-20, 2.0018449164203632e-19, 4.526464619931394e-19, 1.0141572629466542e-18, 2.2515566906928863e-18, 4.9534247195243496e-18, 1.0799081220826378e-17, 2.33313483166002e-17, 4.995485007664705e-17, 1.0600175504069009e-16, 2.229249030249664e-16, 4.646507014857734e-16, 9.59907137799951e-16, 1.9655241393046614e-15, 3.989199880363899e-15, 8.025331524026197e-15, 1.6003731284753996e-14, 3.163528277218813e-14, 6.199052520099292e-14, 1.2041837653985982e-13, 2.3189138796533007e-13, 4.427017406610846e-13, 8.378818255449907e-13, 1.5722052232136343e-12, 2.9248287113415653e-12, 5.394684067585554e-12, 9.865416720280764e-12, 1.7887843503805782e-11, 3.2159019195366675e-11, 5.732694726130582e-11, 1.0132925272674055e-10, 1.7759858273611517e-10, 3.086606384451199e-10, 5.31947057745845e-10, 9.09094707152952e-10, 1.5406762931750028e-09, 2.589304136697256e-09, 4.315506894495427e-09, 7.1328844525597986e-09, 1.169204771089699e-08, 1.900707243258639e-08, 3.0644055554578054e-08, 4.899937817102583e-08, 7.770608457425308e-08, 1.2222112799869957e-07, 1.9066495967797132e-07, 2.950089674619357e-07, 4.5273653422376275e-07, 6.891408328824763e-07, 1.0404675319990329e-06, 1.5581635723107467e-06, 2.31455365595674e-06, 3.4103326814821537e-06, 4.984332380627762e-06, 7.226089527895752e-06, 1.0391804940116747e-05, 1.4824328374289768e-05, 2.0977823171164766e-05, 2.9447742385813452e-05, 4.100666930360939e-05, 5.664642224731157e-05, 7.762657863520474e-05, 0.00010552921980361936, 0.00014231922303790866, 0.00019040882351647142, 0.0002527244384854984, 0.0003327729031641631, 0.000434703341971204, 0.000563359936455955, 0.0007243199183005136, 0.0009239102957877661, 0.0011691962150234562, 0.001467933573928128, 0.0018284786622613524, 0.002259648303143942, 0.002770525310811268, 0.0033702061140171704, 0.004067490137606929, 0.004870513941597997, 0.00578633707591557, 0.006820490936291969, 0.007976506349222812, 0.009255439856693136, 0.01065542235602487, 0.01217125649872297, 0.013794090698552698, 0.015511197424513614, 0.017305881424044114, 0.019157539518797808, 0.021041887668187754, 0.022931363295535228, 0.024795701774847034, 0.02660267599325694, 0.028318977670241257, 0.029911209346479724, 0.03134694739511075, 0.03259582976144983, 0.03363061800784506, 0.03442818207127221, 0.0349703581668828, 0.03524463548583874, 0.03524463548583874, 0.0349703581668828, 0.03442818207127221, 0.03363061800784506, 0.03259582976144983, 0.03134694739511075, 0.029911209346479724, 0.028318977670241257, 0.02660267599325694, 0.024795701774847034, 0.022931363295535228, 0.021041887668187754, 0.019157539518797808, 0.017305881424044114, 0.015511197424513614, 0.013794090698552698, 0.01217125649872297, 0.01065542235602487, 0.009255439856693136, 0.007976506349222812, 0.006820490936291969, 0.00578633707591557, 0.004870513941597997, 0.004067490137606929, 0.0033702061140171704, 0.002770525310811268, 0.002259648303143942, 0.0018284786622613524, 0.001467933573928128, 0.0011691962150234562, 0.0009239102957877661, 0.0007243199183005136, 0.000563359936455955, 0.000434703341971204, 0.0003327729031641631, 0.0002527244384854984, 0.00019040882351647142, 0.00014231922303790866, 0.00010552921980361936, 7.762657863520474e-05, 5.664642224731157e-05, 4.100666930360939e-05, 2.9447742385813452e-05, 2.0977823171164766e-05, 1.4824328374289768e-05, 1.0391804940116747e-05, 7.226089527895752e-06, 4.984332380627762e-06, 3.4103326814821537e-06, 2.31455365595674e-06, 1.5581635723107467e-06, 1.0404675319990329e-06, 6.891408328824763e-07, 4.5273653422376275e-07, 2.950089674619357e-07, 1.9066495967797132e-07, 1.2222112799869957e-07, 7.770608457425308e-08, 4.899937817102583e-08, 3.0644055554578054e-08, 1.900707243258639e-08, 1.169204771089699e-08, 7.1328844525597986e-09, 4.315506894495427e-09, 2.589304136697256e-09, 1.5406762931750028e-09, 9.09094707152952e-10, 5.31947057745845e-10, 3.086606384451199e-10, 1.7759858273611517e-10, 1.0132925272674055e-10, 5.732694726130582e-11, 3.2159019195366675e-11, 1.7887843503805782e-11, 9.865416720280764e-12, 5.394684067585554e-12, 2.9248287113415653e-12, 1.5722052232136343e-12, 8.378818255449907e-13, 4.427017406610846e-13, 2.3189138796533007e-13, 1.2041837653985982e-13, 6.199052520099292e-14, 3.163528277218813e-14, 1.6003731284753996e-14, 8.025331524026197e-15, 3.989199880363899e-15, 1.9655241393046614e-15, 9.59907137799951e-16, 4.646507014857734e-16, 2.229249030249664e-16, 1.0600175504069009e-16, 4.995485007664705e-17, 2.33313483166002e-17, 1.0799081220826378e-17, 4.9534247195243496e-18, 2.2515566906928863e-18, 1.0141572629466542e-18, 4.526464619931394e-19, 2.0018449164203632e-19, 8.772129409033052e-20, 3.808627614566171e-20, 1.638348191740755e-20, 6.982375301847786e-21, 2.948114016335732e-21, 1.2331446439520652e-21, 5.109715375491983e-22, 2.0973762836041472e-22, 8.527793680588291e-23, 3.434481290538298e-23, 1.37003898475025e-23, 5.4129605664519416e-24, 2.1181150042638032e-24, 8.2084131601551185e-25, 3.1502558614649374e-25, 1.1972670524705017e-25, 4.505843745856727e-26, 1.679121395909075e-26, 6.195688573140438e-27, 2.26349155872064e-27, 8.18709712728742e-28, 2.931719130461013e-28, 1.0392866758777137e-28, 3.64710099978195e-29, 1.26688771571373e-29, 4.355965636706001e-30, 1.482396682648639e-30, 4.992928774456252e-31, 1.6643095914854172e-31, 5.490060210873974e-32, 1.7920921931868412e-32, 5.7884114767016835e-33, 1.8499046987397132e-33, 5.84931305771169e-34, 1.8297851103610926e-34, 5.662506351756834e-35, 1.733420311762296e-35, 5.2487790610614054e-36, 1.571969363465091e-36, 4.656213051276345e-37, 1.3639411968385255e-37, 3.9509631646456024e-38, 1.1316829165065295e-38, 3.205016781083655e-39, 8.974046987034233e-40, 2.4840878193536156e-40, 6.797255227087009e-41, 1.8384635725868085e-41, 4.9147045999845377e-42, 1.2984528202428286e-42, 3.3900492351167446e-43, 8.745827265043198e-44, 2.229328518540423e-44, 5.614201403659257e-45, 1.3967037638371808e-45, 3.432289054685043e-46, 8.3307986764200065e-47, 1.9969711112968055e-47, 4.727129683746061e-48, 1.1048953718635373e-48, 2.549758550454317e-49, 5.808802453073383e-50, 1.3062857191121962e-50, 2.89939312356645e-51, 6.3510516040027e-52, 1.3727926269934576e-52, 2.9277567874268057e-53, 6.160055652032759e-54, 1.2785021164596291e-54, 2.6171690383997113e-55, 5.283486791135567e-56, 1.0517479560808506e-56, 2.064178231560548e-57, 3.993631543578683e-58, 7.615762478452372e-59, 1.4312685864376847e-59, 2.6504973822920087e-60, 4.835780443442695e-61, 8.691033976694244e-62, 1.538412910817142e-62, 2.6816371839931834e-63, 4.6023521464413906e-64, 7.775663443759427e-65, 1.2929918710579457e-65, 2.115804879913002e-66, 3.406397879225015e-67, 5.394747772528304e-68, 8.402654544118577e-69, 1.2868930382884307e-69, 1.937569293602806e-70, 2.8672550084705196e-71, 4.1693864776416954e-72, 5.956266396630994e-73, 8.357344832689367e-74, 1.151456399170535e-74, 1.557402225042187e-75, 2.0673480863391865e-76, 2.692572562781722e-77, 3.4398504106021997e-78, 4.309263151743415e-79, 5.2920775547726144e-80, 6.369021127188049e-81, 7.509326219828704e-82, 8.670899556664952e-83, 9.80188645536038e-84, 1.0843735557990875e-84, 1.1735644543280168e-85, 1.2420012583601042e-86, 1.2848288879587286e-87, 1.2986442523453815e-88, 1.2819235108988745e-89, 1.2352582010802858e-90, 1.1613538642635167e-91, 1.0647807284292371e-92, 9.515061828516586e-94, 8.282750211659874e-95, 7.019279840389723e-96, 5.78756688742493e-97, 4.6398215553195645e-98, 3.61417679045945e-99, 2.7334110179945424e-100, 2.005647497480272e-101, 1.426611190676344e-102, 9.828427827206546e-104, 6.552285218137697e-105, 4.222886523124088e-106, 2.6283526077535817e-107, 1.5780999094172644e-108, 9.12950360819905e-110, 5.082404070543801e-111, 2.71898160152549e-112, 1.3957811096126747e-113, 6.864497260390203e-115, 3.2287001429238176e-116, 1.4496204723331426e-117, 6.200006093481872e-119, 2.520327680277184e-120, 9.713230410804562e-122, 3.539233752924739e-123, 1.2154944201963751e-124, 3.920949742568952e-126, 1.18338523417574e-127, 3.3267857988876226e-129, 8.666977031170159e-131, 2.080074487480838e-132, 4.56702981283218e-134, 9.0976689498649e-136, 1.6278135297969005e-137, 2.58383099967762e-139, 3.5815479203452157e-141, 4.246894767207766e-143, 4.1882591392581515e-145, 3.2978418419355524e-147, 1.9437181779580859e-149, 7.622424227286611e-152, 1.4916681462400413e-154
  };
  
  // Shift
  for (unsigned int i = filter_order-1; i >= 1; --i) {
    delta_t[i] = delta_t[i-1];
  }  
  delta_t[0] = raw_delta_t;
  
  // Summing up. TODO: This can be optimized in regards to rounding
  // errors. Just sum up the small coefficients first.
  for (unsigned int i = 0; i < filter_order; ++i) {
    result += coeffs[i] * delta_t[i];
  }
  return result;
}