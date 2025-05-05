#ifndef ICONS_H
#define ICONS_H

const double I = 0.1; //Intensidade de cor

double icon_zero[25] = { //Desenho do número 0
    I, 0, 0, 0, I,
    0, I, 0, I, 0,
    0, 0, I, 0, 0,
    0, I, 0, I, 0,
    I, 0, 0, 0, I
};

double icon_one[25] = { //Desenho do número 1
    0, 0, 0, 0, 0,
    0, I, I, I, 0,
    0, I, 0, I, 0,
    0, I, I, I, 0,
    0, 0, 0, 0, 0
};
double icon_two[25] = { //Desenho do número 2
    0, 0, I, 0, 0,
    0, 0, I, 0, 0,
    0, 0, I, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, I, 0, 0
};
double icon_three[25] = { //Desenho do número 3
    0, I, I, I, 0,
    0, 0, I, I, I,
    I, I, 0, 0, 0,
    0, 0, I, I, I,
    0, I, I, I, 0
};

#endif // ICONS_H