# Filtering Modeling Medical Images

Projet IRM

I. Problem Presentation

The Object of this paper is to present an image processing chip for a MRI system. It will be implemented on the VirtualSoC platform and will use an hardware accelerator. The image processing deals with 129x96 grayscale images of brain sections. The prossessing has to facilitate tumor detection by giving away its contour. The process includes three steps. First, il will reduce the noise of the picture with a median filter. Then it will use a threshold filter in order to do a segmentation of the picture. Then, a Sobel filter will be applied to show the contour. The time target is fixed to one millisecond for the whole process.


II. First implementation

a) Median Filter

The median filter takes all the eight neighbours of a point and the pixel itself and return the median value for the central pixel. As a first implementation, a sort algorithm like a quicksort can be used : 

b) Threshold

The Threshold filter is the most simple and the fastest part of the process. It only use a comparison to a threshold value for each pixel. The law is : all the pixels that have a value smaller than 100 equal zero.

c) Sobel Filter

The Sobel Filter return for each pixels the sum of the absolute values of two gradients computed with the masks presented on figure 2. 


III. Coding optimisation

a) Local memory use



b) Median Filter

c) Threshold

Loop Unrolling

d) Sobel Filter
” Reduce non useful computations


IV. Parallelisation of the process

a) Open MP

b ) performances

Parallelisation process was very efficient with the threshold and the Sobel Filter. 

V. Material acceleration

Balance

The time target is one milisecond for the whole process for a 126*96 picture. At this stage, the threshold lasts 0.05ms and Sobel filter 0,4ms, and median filter 10ms which is the critial process. However, the read/write process of the picture from the bus lasts 0.4ms. It means that system including a hardware implementation of the median filter as fast as the read/write process reach the time target.


Choice of the strategy


Sort acceleration is a large problem with an abundant literature. However, this paper deals only with a median filter using the eight neigbourghs of each pixel. It means that a efficient sort algorithm for 9 values is usefull.

To make the system reliable, it is possible to only use a very stable sort algorithm.

The next section present the different steps of the hardware accelerator. All of them are pipelined :

Step1 : image buffering.
The median filter needs to work on a 3x3 area. So we will use a buffer of 2x IMAGE_X +3 pixels to have a direct access to these 9 pixels at each pixel reception. Unfortunately,, it increases the time process of this number of pixel reading. However it is a small value.This step also send 

Step 2

Balance

V. Conclusion and suggested future optimisations

