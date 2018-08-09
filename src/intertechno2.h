// 

#include "TabRF.h"

#define IT2_LEN 275

Code Intertechno2a_Codes[] = {
    // starting sequence is /‾\___________________/
    {TabRF_CODE_START, 's', 2, {IT2_LEN, 10 * IT2_LEN}},

    // low signal is  /‾\_/‾\_________/
    {TabRF_CODE_ANY, '_', 4, {IT2_LEN, IT2_LEN, IT2_LEN, 5 * IT2_LEN}},

    // high signal is /‾\_________/‾\_/
    {TabRF_CODE_ANY, '#', 4, {IT2_LEN, 5 * IT2_LEN, IT2_LEN, IT2_LEN}},

    // DIM signal is /‾\_/‾\_/  (shorter !)
    {TabRF_CODE_ANY, 'D', 4, {IT2_LEN, IT2_LEN, IT2_LEN, IT2_LEN}},

    // end signal is /‾\_______40 times______________________...__/  (longer !)
    {TabRF_CODE_END, 'x', 2, {IT2_LEN, 10728}}
};


Code Intertechno2_Codes[] = {
    // starting sequence is /‾\__________/
    {TabRF_CODE_START, 's', 2, {275, 2675}},

    // low signal is /‾\_/‾\_____/
    {TabRF_CODE_ANY, '_', 4, {275, 275, 275, 1225}},

    // high signal is /‾\_____/‾\_/
    {TabRF_CODE_ANY, '#', 4, {275, 1225, 275, 275}},

    // DIM signal is /‾\_/‾\_/  (shorter !)
    {TabRF_CODE_ANY, 'D', 4, {275, 275, 275, 275}},

    // end signal is /‾\_/‾\______40 times____/  (longer !)
    {TabRF_CODE_END, 'x', 2, {275, 9900}}
};


Sequence Intertechno2_Sequence = {
  8,  // min. Code length
  38, // max code length
  4,  // tolerance
  3,  // send repeat

  sizeof(Intertechno2_Codes) / sizeof(Code),  // # of code definitions following...

  Intertechno2_Codes
}; // Sequence


