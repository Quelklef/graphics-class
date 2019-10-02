#include <stdio.h>
#include <FPT.h>
//#include "M2d_matrix_tools.c"
/*
void turnPoly(gons[][],amountofPoints[], col[],numPolys,midX,midY){
   int i;
   int j;
   int k;
   double r;

   

   for(i = 0; i < numPolys*2; i += 2){
       for(j = 0; j < amountofPoints[i/2]; j++) {
	   x= rcos(x)
	   y= rsin(y)
	   
	   r = sqrt(pow(midX-gons[i][j],2)+pow(midY - gons[i+1][j],2))
	   gons[i][j] = r*cos(gons[i][j]+.05); 
	   
	 for(k = i; k < M_PI/8; k += 0.05){
     		G_line(x1,y1,x1+radius*cos(k+j),y1+radius*sin(k+j));
               

       }
   }
}
*/

int main(int argc, char **argv) {


 double sWidth = 800;
 double sHeight = 800;
 G_init_graphics (sWidth,sHeight);
  int q = 49;
  while(q-48 != 0){
  G_rgb(1,1,1);
  G_clear();

  FILE *fp;

  fp = fopen(argv[q-48], "r");

  if(fp == NULL) {
    printf("Can not open file: %s\n", argv[2]);
    exit(0);
  }
 int np;
 fscanf(fp, "%d", &np );
 double x[np];
 double y[np];
 int i; 

double maxX = -4000;
double maxY = -4000;
double minX = 4000;
double minY = 4000;

 for(i = 0; i < np; i++){
    fscanf(fp,"%lf %lf" , &x[i], &y[i]);
    if(x[i] > maxX){maxX = x[i];}
    else if(x[i] < minX){minX = x[i];}
    if(y[i] > maxY){maxY = y[i];}
    else if(y[i] < minY){minY = y[i];}
 }
printf("%lf %lf %lf %lf\n",maxX,minX,maxY,minY);
double midX = (maxX+minX)/2;
double midY = (maxY+minY)/2;
double scaler;
double var;

if((maxX - minX) > (maxY - minY)) { var = maxX - minX;}
else { var = maxY - minY;} 

  scaler = (.6*sWidth)/var;

  for(i = 0; i < np; i++) {
	x[i] *= scaler;
	y[i] *= scaler;

  }

double diffX = (midX*scaler - sWidth/2);
double diffY = (midY*scaler - sHeight/2);

printf("%lf %lf \n", diffX, diffY);
for(i = 0; i < np; i++){

  x[i] -= diffX;
  y[i] -= diffY;

}	

 int numpoly;

 fscanf(fp, "%d" , &numpoly);

 //double r;

  //fscanf(fp, "%lf" , &r);
  //printf("%lf", r);
 
printf("hi\n");

  int j;
  int polygonpoint[100];
  double polygons[100][100];
  int holder;
 for(i = 0; i < numpoly*2; i+=2) {
   fscanf(fp, "%d", &polygonpoint[i/2]);
   printf(" Poly: %d\n", i/2);
   for(j = 0; j < polygonpoint[i/2]; j++) {
     fscanf(fp, "%d" , &holder);
     polygons[i][j] = x[holder];
     polygons[i+1][j] = y[holder];
     

   }
 }  
 printf("bill\n");
 double r,g,b;
 double colors[numpoly][3];
for(i = 0; i < numpoly*2; i+=2){

   fscanf(fp, "%lf %lf %lf", &r, &g, &b);

    G_rgb(r,g,b);
    colors[i/2][0] = r;
    colors[i/2][1] = g;
    colors[i/2][2] = b;
    G_fill_polygon(polygons[i],polygons[i+1],polygonpoint[i/2]);


 }
 



q =  G_wait_key();
}
  
}


