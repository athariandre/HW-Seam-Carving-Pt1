#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include "functions.h"

using std::cout, std::endl, std::string, std::cin;

void initializeImage(Pixel image[][MAX_HEIGHT]) {
  // iterate through columns
  for (unsigned int col = 0; col < MAX_WIDTH; col++) {
    // iterate through rows
    for (unsigned int row = 0; row < MAX_HEIGHT; row++) {
      // initialize pixel
      image[col][row] = {0, 0, 0};
    }
  }
}

void loadImage(string filename, Pixel image[][MAX_HEIGHT], unsigned int& width, unsigned int& height) {
  std::ifstream inFS;
  string filetype;
  int maxcolorval;
  short pr;
  short pg;
  short pb;
  int garbage;

  inFS.open(filename);

  if(!inFS.is_open()){
    throw std::runtime_error("Failed to open " + filename);
  }

  inFS >> filetype;

  if(filetype != "P3" && filetype != "p3"){
    throw std::runtime_error("Invalid type " + filetype);
  }
  
  inFS >> width >> height >> maxcolorval;

  if(inFS.fail() || height > MAX_HEIGHT || height <= 0 || width > MAX_WIDTH || width <= 0 || maxcolorval != 255){
    throw std::runtime_error("Invalid dimensions");
  }


  for(unsigned int i = 0; i < height; i++){
    for(unsigned int j = 0; j < width; j++){

      inFS >> pr >> pg >> pb;

      if (inFS.fail() || pr >= 256 || pr < 0 || pg >= 256 || pg < 0 || pb >= 256 || pb < 0) {
        throw std::runtime_error("Invalid color value");
      }


      Pixel p = {pr,pg,pb};
      image[j][i] = p;  
    }
  }
  

  inFS >> garbage;
  if(!inFS.fail()){
    throw std::runtime_error("Too many values");
  }
}


void outputImage(string filename, Pixel image[][MAX_HEIGHT], unsigned int width, unsigned int height) {
  std::ofstream outFS;
  outFS.open(filename);
  if(!outFS.is_open()){
    throw std::runtime_error("Failed to open " + filename);
  }

  outFS << "P3" << endl;
  outFS << width << " " << height << endl;
  outFS << "255" << endl;
  for(unsigned int i = 0; i < height; i++){
    for(unsigned int j = 0; j < width; j++){
      Pixel p = image[j][i];
      outFS << p.r << " " << p.g << " " << p.b << " ";
    }
    outFS << endl;
  }
}

unsigned int energy(Pixel image[][MAX_HEIGHT], unsigned int x, unsigned int y, unsigned int width, unsigned int height) {


  int energy_total;
  int rx, bx, gx, ry, by, gy;
  int xleft, xright, yup, ydown;

  int xmax = width-1;
  int ymax = height-1;

  xleft = x-1;
  xright = x+1;
  yup = y+1;
  ydown = y-1;

  if(xleft < 0){
    xleft = xmax;
  }
  if(xright > xmax){
    xright = 0;
  }
  if(ydown < 0){
    ydown = ymax;
  }
  if(yup > ymax){
    yup = 0;
  }

  
  rx = image[xright][y].r - image[xleft][y].r;
  gx = image[xright][y].g - image[xleft][y].g;
  bx = image[xright][y].b - image[xleft][y].b;

  ry = image[x][yup].r - image[x][ydown].r;
  gy = image[x][yup].g - image[x][ydown].g;
  by = image[x][yup].b - image[x][ydown].b;


  rx*=rx;
  gx*=gx;
  bx*=bx;
  ry*=ry;
  gy*=gy;
  by*=by;

  energy_total = (rx+gx+bx+ry+by+gy);

  return (unsigned int)energy_total;
}


unsigned int loadVerticalSeam(Pixel image[][MAX_HEIGHT], unsigned int start_col, unsigned int width, unsigned int height, unsigned int seam[]) {
  unsigned int curr_col;
  int l_col, r_col;
  unsigned int leastEnergyCol;

  curr_col = start_col;

  seam[0] = curr_col;

  for(unsigned int i = 1; i < height; i++){
    leastEnergyCol = curr_col;
    l_col = curr_col + 1;
    r_col = curr_col - 1;

    if(l_col <= (int)width-1){
      if(energy(image, l_col, i, width, height) < energy(image, leastEnergyCol, i, width, height)){
        leastEnergyCol = l_col;
      }
    }

    if(r_col >= 0){
      if(energy(image, r_col, i, width, height) < energy(image, leastEnergyCol, i, width, height)){
        leastEnergyCol = r_col;
      }
    }
    
    seam[i] = leastEnergyCol;
    curr_col = leastEnergyCol;
  }
  unsigned int seamEnergy = 0;
  for(unsigned int i = 0; i < height; i++){
    seamEnergy += (energy(image, seam[i], i, width, height));
  }

  return seamEnergy;
}

unsigned int loadHorizontalSeam(Pixel image[][MAX_HEIGHT], unsigned int start_row, unsigned int width, unsigned int height, unsigned int seam[]) {
  unsigned int curr_row;
  int l_row, r_row;
  unsigned int leastEnergyRow;

  curr_row = start_row;

  seam[0] = curr_row;

  for(unsigned int i = 1; i < width; i++){
    leastEnergyRow = curr_row;
    l_row = curr_row - 1;
    r_row = curr_row + 1;

    if(l_row >= 0){
      if(energy(image, i, l_row, width, height) < energy(image, i, leastEnergyRow, width, height)){
        leastEnergyRow = l_row;
      }
    }

    if(r_row <= (int)height-1){
      if(energy(image, i, r_row, width, height) < energy(image, i, leastEnergyRow, width, height)){
        leastEnergyRow = r_row;
      }
    }
    seam[i] = leastEnergyRow;
    curr_row = leastEnergyRow;
  }

  unsigned int seamEnergy = 0;
  for(unsigned int i = 0; i < width; i++){
    seamEnergy += (energy(image, i, seam[i], width, height));
  }

  return 0;
}

void findMinVerticalSeam(Pixel image[][MAX_HEIGHT], unsigned int width, unsigned int height, unsigned int seam[]) {
  int smallestcol = width-1;
  for(int j = width-1; j >= 0; j--){
    if(loadVerticalSeam(image, j, width, height, seam) <= loadVerticalSeam(image, smallestcol, width, height, seam)){
      smallestcol = j;
    }
  }
  loadVerticalSeam(image, smallestcol, width, height, seam); //re-update seam
}

void findMinHorizontalSeam(Pixel image[][MAX_HEIGHT], unsigned int width, unsigned int height, unsigned int seam[]) {
  int smallestrow = height-1;
  for(int j = height-1; j >= 0; j--){
    if(loadHorizontalSeam(image, j, width, height, seam) <= loadHorizontalSeam(image, smallestrow, width, height, seam)){
      smallestrow = j;
    }
  }
  loadHorizontalSeam(image, smallestrow, width, height, seam); //re-update seam
}

void removeVerticalSeam(Pixel image[][MAX_HEIGHT], unsigned int& width, unsigned int height, unsigned int verticalSeam[]) {
  int targetcol;
  for(unsigned int i = 0; i < height; i++){
    targetcol = verticalSeam[i];
    for(unsigned int j = targetcol; j < width-1; j++){
      image[j][i] = image[j+1][i];
    }
  }
  width--;
}

void removeHorizontalSeam(Pixel image[][MAX_HEIGHT], unsigned int width, unsigned int& height, unsigned int horizontalSeam[]) {
  int targetrow;
  for(unsigned int i = 0; i < width; i++){
    targetrow = horizontalSeam[i];
    for(unsigned int j = targetrow; j < height-1; j++){
      image[i][j] = image[i][j+1];
    }
  }
  height--;
}
