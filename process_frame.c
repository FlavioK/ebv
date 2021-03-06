/* Copying and distribution of this file, with or without modification,
 * are permitted in any medium without royalty. This file is offered as-is,
 * without any warranty.
 */

/*! @file process_frame.c
 * @brief Contains the actual algorithm and calculations.
 */


/* Definitions specific to this application. Also includes the Oscar main header file. */
#include "template.h"
#include <string.h>
#include <stdlib.h>

#define IMG_SIZE NUM_COLORS*(OSC_CAM_MAX_IMAGE_WIDTH/2)*(OSC_CAM_MAX_IMAGE_HEIGHT/2)
#define SLOW
#define SENSITIVITY 15
const int nc = OSC_CAM_MAX_IMAGE_WIDTH/2;
const int nr = OSC_CAM_MAX_IMAGE_HEIGHT/2;
const int Border = 6;

int avgDxy[3][IMG_SIZE];


int TextColor;

int myVal = 0;

void CalcDeriv(void);
void AvgDeriv(int Index);
void calculateMC(void);
void findMax(void);
void drawBoxes(void);


void ResetProcess()
{
	//called when "reset" button is pressed
	if(TextColor == CYAN)
		TextColor = MAGENTA;
	else
		TextColor = CYAN;
}


void ProcessFrame()
{

	uint32 t1, t2;
	//initialize counters
	if(data.ipc.state.nStepCounter == 1) {
		//use for initialization; only done in first step
		memset(data.u8TempImage[THRESHOLD], 0, IMG_SIZE);
        memset(avgDxy,0,sizeof(avgDxy)); // init memory
		TextColor = CYAN;
	} else {
		//example for time measurement
		t1 = OscSupCycGet();
		//example for copying sensor image to background image
    	//	memcpy(data.u8TempImage[BACKGROUND], data.u8TempImage[SENSORIMG], IMG_SIZE);
		//example for time measurement

        CalcDeriv();
        calculateMC();
        findMax();
        drawBoxes();
        t2 = OscSupCycGet();
	//example for log output to console
		OscLog(INFO, "required = %d us\n", OscSupCycToMicroSecs(t2-t1));

        }
   

}


void CalcDeriv(void)
{
    int c,r;
    for(r = nc; r<nr*nc-nc;r+=nc){/*we skip the first and last line*/
        for(c=1;c<nc-1;c++){
            /*do pointer arithmetics with respect to center pixel location*/
            unsigned char* p = &data.u8TempImage[SENSORIMG][r+c];
            /*implement Sobel filter*/

            int dx = -(int) *(p-nc-1) + (int) *(p-nc+1)
                    -2 * (int) *(p-1)  + 2 * (int) *(p+1)
                    -(int) *(p+nc-1) + (int) *(p+nc+1);

            
            int dy = -(int) *(p-nc-1) - 2 * (int) *(p-nc) -(int) *(p-nc+1)
                      +(int) *((p+nc)-1) +2 * (int) *(p+nc) +(int) *(p+nc+1);
    
            avgDxy[0][r+c] = dx*dx;
            avgDxy[1][r+c] = dy*dy;   
            avgDxy[2][r+c] = dx*dy;
        }
    }
}

#ifdef SLOW


void AvgDeriv(int Index)
{
    //do average in x-direction
    int c,r;
    int helpBuf[IMG_SIZE];

   for(r=nc;r<nr*nc-nc; r+=nc){ /*we skip first and last lines (empty)*/
        for(c=Border+1; c<nc-(Border+1);c++){/*+1 because we have one empty border column*/
            /*do pointer arithmetics with respect to center pixel location*/
            int *p = &avgDxy[Index][r+c];
            int sx = (*(p-6) + *(p+6))*1 + (*(p-5) + *(p+5))*4 + (*(p-4) + *(p+4))*11 +
                (*(p-3) + *(p+3))*27 + (*(p-2) + *(p+2))*50 + (*(p-1) + *(p+1))*72 + (*p)*82;
            //now averaged
            helpBuf[r+c] = (sx >>8);
            
        }
    }

     for(r=nc*(Border+1);r<nr*nc-nc*(Border+1); r+=nc){ /*we skip the border lines*/
        for(c=Border+1; c<nc-(Border+1);c++){/*+1 because we have one empty border column*/
            /*do pointer arithmetics with respect to center pixel location*/
            int *p = &helpBuf[r+c];
            int sy = (*(p-6*nc) + *(p+6*nc))*1 + (*(p-5*nc) + *(p+5*nc))*4 + (*(p-4*nc) + *(p+4*nc))*11 +
                (*(p-3*nc) + *(p+3*nc))*27 + (*(p-2*nc) + *(p+2*nc))*50 + (*(p-1*nc) + *(p+1*nc))*72 + (*p)*82;
            //now averaged
            avgDxy[Index][r+c] = (sy>>8);          
        }
    }
}            
#else

void AvgDeriv(int Index)
{
    //do average in x-direction
    int c,r;
    int helpBuf[IMG_SIZE];

    for(r=nc;r<nr*nc-nc; r+=nc){ /*we skip first and last lines (empty)*/
        for(c=Border+1; c<nc-(Border+1);c++){/*+1 because we have one empty border column*/
            /*do pointer arithmetics with respect to center pixel location*/
            int *p = &avgDxy[Index][r+c];
            int sx = (*(p-6) + *(p+6)) + ((*(p-5) + *(p+5)) << 2) + ((*(p-4) + *(p+4)) << 3) + 
                        ((*(p-3) + *(p+3)) << 5) + ((*(p-2) + *(p+2)) << 6) + ((*(p-1) + *(p+1)) << 6) + (*p << 7);
               
            //now averaged
            helpBuf[r+c] = (sx >>8);
            
        }
    
    }

     for(r=nc*(Border+1);r<nr*nc-nc*(Border+1); r+=nc){ /*we skip the border lines*/
        for(c=Border+1; c<nc-(Border+1);c++){/*+1 because we have one empty border column*/
            /*do pointer arithmetics with respect to center pixel location*/
            int *p = &helpBuf[r+c];
            int sy =  (*(p-6*nc) + *(p+6*nc)) + ((*(p-5*nc) + *(p+5*nc))<<2) + ((*(p-4*nc) + *(p+4*nc))<<3) +
                ((*(p-3*nc) + *(p+3*nc))<<5) + ((*(p-2*nc) + *(p+2*nc)) <<6) + ((*(p-1*nc) + *(p+1*nc))<<6) + (*p<<7);
            //now averaged
            avgDxy[Index][r+c] = (sy>>8);            
        }
    }   
}
#endif

void calculateMC(void)
{
    int c,r = 0;
   
    /*Build mean values*/
    AvgDeriv(0);
    AvgDeriv(1);
    AvgDeriv(2);
    
    
    /*Calculate MC*/
    for(r=nc *(Border +1);r<nr*nc -nc *(Border + 1);r+= nc){
        for(c = Border +1;c< nc- (Border +1);c++){
            int dIx2 = (avgDxy[0][r+c] >> 7);
            int dIy2 = (avgDxy[1][r+c] >> 7);
            int dIxy = (avgDxy[2][r+c] >> 7);

            avgDxy[2][r+c] = ((dIx2*dIy2)-dIxy*dIxy)-((5*(dIx2+dIy2)*(dIx2+dIy2))>>7);
//          data.u8TempImage[BACKGROUND][r+c] = MAX(0,MIN(255,(avgDxy[2][r+c]>>10))); 
   
         }
    } 
}

void findMax(void){

    int c,r = 0;

    /*Find Max in x direction*/
    for(r=nc *(Border +1);r<nr*nc -nc *(Border + 1);r+= nc){
        for(c = Border +1;c< nc- (Border +1);c++){
             /*do pointer arithmetics with respect to center pixel location*/
            int *p = &avgDxy[2][r+c];
            int tmp = avgDxy[2][r+c];
            int cm,rm;//coulumn mask and row mask

            for(cm = -6;cm<7;cm++){
                for(rm = -6;rm<7;rm++){
                    if(tmp < *((p+rm)+nc*cm)){
                        avgDxy[0][r+c] = 0;
                      
                        rm = 8;
                        cm = 8;
                    }else{
                        avgDxy[0][r+c]=tmp;
                    }
                }

            }

//      data.u8TempImage[BACKGROUND][r+c] = MAX(0,MIN(255,(avgDxy[0][r+c]>>10))); 
        }
    }   
}

void drawBoxes(void)
{
    int r,c = 0;
    int sizeBox = 5;
    for(r = nc*(Border +1); r < (nr*nc - nc*(Border+1)); r+= nc){
        for(c = Border+1; c < (nc - (Border+1)); c++){       
             if((avgDxy[0][r+c]>>SENSITIVITY) > (255/100*data.ipc.state.nThreshold)){
                 avgDxy[1][r+c] = 255;
                 DrawBoundingBox(c - sizeBox,
                                 (r/nc)-sizeBox,
                                 c+sizeBox,
                                 (r/nc)+sizeBox,
                                 false,
                                 GREEN);
                 printf("POS: %i \t%i\n", r/(nc), c);
                 
             }else{
                 avgDxy[1][r+c]=0;
               
             }
              data.u8TempImage[BACKGROUND][r+c] = MAX(0,MIN(255,(avgDxy[1][r+c])));          
        }
    }
  
}




