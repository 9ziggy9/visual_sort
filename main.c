// Suppress usleep() usage warning
// #define _XOPEN_SOURCE 500
// I'm keeping this here just in case I reimplement beeping and need it.

// SDL2 libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>
// Standard libraries
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
// Audio: toot. Thank you very much, Guillaume Vareille!
// Please see toot.h/toot.c for further information..
#include "toot.h"
// MACROS
#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 640
#define MAX_NUMBER 641

bool running;

SDL_Renderer* rend;
SDL_Window* win;

int frameCount, lastTime, timerFPS, lastFrame, fps;

void input() 
{
	SDL_Event e;
	while(SDL_PollEvent(&e))
		if(e.type == SDL_QUIT)
			running = false;
	const Uint8 *keystates = SDL_GetKeyboardState(NULL);
	if(keystates[SDL_SCANCODE_ESCAPE])
		running = false;
}

void render_backsplash()
{
	// backsplash of window, gruxbox gray
	SDL_SetRenderDrawColor(rend, 40, 40, 40, 255);
	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = WINDOW_WIDTH;
	rect.h = WINDOW_HEIGHT;
	SDL_RenderFillRect(rend, &rect);
}

void populate_segment_height(int *array, int NUM)
{
	int step = WINDOW_HEIGHT / NUM;

	for (int n=0; n<NUM; n++)
		array[n] = step * (n+1);
}

void swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void unhighlight_segments(bool *hl, int NUM)
{
	for(int j=0; j<NUM; j++)
		hl[j] = false;
}

void shuffle_segments(int *array, int n)
{
	// Fisher-Yates shuffle implementation
	for (int i = n-1; i>0; i--)
	{
		int j = rand()%(i+1);
		swap(&array[i], &array[j]);
	}
}

void draw_segments(int *array, bool *hl, int NUM)
{
	int SEGMENT_WIDTH = WINDOW_WIDTH / NUM;

	// foreground, segment struct, gruvbox
	SDL_Rect segment;
	SDL_SetRenderDrawColor(rend, 251, 241, 199, 255);
	segment.y = WINDOW_HEIGHT;
	segment.w = SEGMENT_WIDTH;

	for (int n=0; n<NUM; n++) 
	{
		segment.x = n * SEGMENT_WIDTH;
		segment.h = - array[n]; // NOTE: direction of increasing y reversed in SDL
		if (hl[n]) {
			SDL_SetRenderDrawColor(rend, 251, 73, 52, 255);
			SDL_RenderFillRect(rend, &segment); // draw
		} else {
				SDL_SetRenderDrawColor(rend, 251, 241, 199, 255);
			  SDL_RenderFillRect(rend, &segment); // draw
			}
	}
}

void draw_initially(int *array, int NUM)
{
	int SEGMENT_WIDTH = WINDOW_WIDTH / NUM;
	
	render_backsplash();
	
	// foreground, segment struct, gruvbox
	SDL_Rect segment;
	SDL_SetRenderDrawColor(rend, 251, 241, 199, 255);
	segment.y = WINDOW_HEIGHT;
	segment.w = SEGMENT_WIDTH;

	for (int n=0; n<NUM; n++) 
	{
		segment.x = n * SEGMENT_WIDTH;
		segment.h = - array[n]; // NOTE: direction of increasing y reversed in SDL
		SDL_RenderFillRect(rend, &segment); // draw
		SDL_RenderPresent(rend);
		toot(10*array[n], 1);
	}
}

void draw_finally(int *array, int NUM)
{
	int SEGMENT_WIDTH = WINDOW_WIDTH / NUM;
	
	render_backsplash();
	
	// foreground, segment struct, gruvbox
	SDL_Rect segment;
	SDL_SetRenderDrawColor(rend, 102, 255, 0, 255);
	segment.y = WINDOW_HEIGHT;
	segment.w = SEGMENT_WIDTH;

	for (int n=0; n<NUM; n++) 
	{
		segment.x = n * SEGMENT_WIDTH;
		segment.h = - array[n]; // NOTE: direction of increasing y reversed in SDL
		SDL_RenderFillRect(rend, &segment); // draw
		SDL_RenderPresent(rend);
		toot(10*array[n], 1);
	}
}

void draw_state(int *array, bool *hl, int NUM) 
{
	render_backsplash();

	// draw current array sequence
	draw_segments(array, hl, NUM);

	// fps handling
	frameCount++;
	int timerFPS = SDL_GetTicks() - lastFrame;
	if(timerFPS<(1000/60))
		SDL_Delay((1000/60) - timerFPS);

	//draw
	SDL_RenderPresent(rend);
}

/////////////////////////
// SORTING ALGORITHMS //
////////////////////////

// Insertion Sort
void insertionSort(int *array, int NUM) 
{ 
	int i, element, j; 
	bool highlight[MAX_NUMBER] = {false};
	
	for (i = 1; i < NUM; i++)
	{ 
		element = array[i]; 
		j = i - 1; 
		while (j >= 0 && array[j] > element) 
		{ 
			array[j + 1] = array[j]; 
			j = j - 1; 
		} 
		array[j + 1] = element; 
		highlight[j+1] = true;
		draw_state(array, highlight, NUM);
		toot(10*array[j], 20);
		unhighlight_segments(highlight, NUM);
	} 
	unhighlight_segments(highlight, NUM);
	draw_state(array, highlight, NUM);
} 

// Heap Sort
void heapify(int *array, int n, int i) 
{
	int max = i;
	int leftChild = 2 * i + 1;
	int rightChild = 2 * i + 2;

	if (leftChild < n && array[leftChild] > array[max])
		max = leftChild;

	if (rightChild < n && array[rightChild] > array[max])
		max = rightChild;

	if (max != i) {
		swap(&array[i], &array[max]);
		heapify(array, n, max);
	}
}
  
void heapSort(int *array, int n, int NUM) 
{
	bool highlight[MAX_NUMBER] = {false};
    
	for (int i = n / 2 - 1; i >= 0; i--) {
		heapify(array, n, i);
		highlight[i] = true;
		draw_state(array, highlight, NUM);
		toot(9*array[i], 20);
		unhighlight_segments(highlight, NUM);
	}
  
  for (int i = n - 1; i >= 0; i--) {
    swap(&array[0], &array[i]);
    heapify(array, i, 0);
		highlight[i] = true;
		draw_state(array, highlight, NUM);
		toot(10*array[i], 20);
		unhighlight_segments(highlight, NUM);
  }
}

// Quicksort
void quickSort(int *array, int first, int last, int NUM) 
{
	int i, j, pivot, tmp;
	bool highlight[MAX_NUMBER] = {false};

	if(first<last) {
		pivot=first;
		i=first;
		j=last;
		while(i<j) {
			while(array[i]<=array[pivot]&&i<last)
				i++;
			while(array[j]>array[pivot])
				j--;
			
			if(i<j) {
				tmp=array[i];
				array[i]=array[j];
				array[j]=tmp;
			}
		}
		tmp=array[pivot];
		array[pivot]=array[j];
		highlight[pivot] = true;
		array[j]=tmp;
		draw_state(array, highlight, NUM);
		toot(10*array[i], 20);
		unhighlight_segments(highlight, NUM);

		quickSort(array, first, j-1, NUM);
		highlight[first] = true;
		draw_state(array, highlight, NUM);
		toot(10*array[i], 20);
		unhighlight_segments(highlight, NUM);
		
		quickSort(array, j+1, last, NUM);
		highlight[last] = true;
		draw_state(array, highlight, NUM);
		toot(10*array[i], 20);
		unhighlight_segments(highlight, NUM);
	}
}

int main(int argc, char *argv[])
{
	running = true; // running state is acted upon by input()
	bool SORT_COMPLETE = false;
	int SEGMENT_NUMBER = -1;
	int SORT = -1;
	int opt;

	while ((opt = getopt(argc, argv, "n:s:h")) != -1)
		switch (opt)
		{
			case 'n':
				SEGMENT_NUMBER = atoi(optarg);
				break;
			case 's':
				SORT = atoi(optarg);
				break;
			case 'h':
				printf("\nPass number of segments with -n <number>\n");
				printf("Pass sort choice with -s <option>\n");
				printf("1. Insertion sort.\n");
				printf("2. Quicksort.\n");
				printf("3. Heap sort.\n");
				printf("Example: vsort -n 192 -s 3\n");
				exit(0);
			case '?':
				break;
		}

	if (SORT == -1) {
		printf("\nYOU DID NOT SPECIFY A SORTING METHOD.\n");
		printf("1. Insertion sort.\n");
		printf("2. Quicksort.\n");
		printf("3. Heap sort.\n");
		printf("Sort selection: ");
		scanf("%d", &SORT);
	}

	if (SEGMENT_NUMBER == -1) {
		printf("\nYOU DID NOT SPECIFY NUMBER OF SEGMENTS.\n");
		printf("Please specify a number between 2 and 640.\n");
		printf("Number selection: ");
		scanf("%d", &SEGMENT_NUMBER);
	}

	int segment_height[SEGMENT_NUMBER]; // array of segment arrays, primary argument

	printf("\nNumber of segments: %d\n", SEGMENT_NUMBER);
	printf("Algorithm:");

	switch (SORT)
	{
		case 1:
			printf(" insertion sort.");
			break;
		case 2:
			printf(" quicksort.");
			break;
		case 3:
			printf(" heap sort.");
			break;
	}
	
	printf("\n");
	
	// SDL2 initialization
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		printf("Failed at SDL_Init()\n");
	if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &win, &rend) < 0)
		printf("Failed at SDL_CreateWindowAndRenderer()\n");
	SDL_SetWindowTitle(win, "Visual Sort");
	SDL_ShowCursor(0);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	// INITIAL CONDITIONS: Seed RNG, populate array and apply Fisher-Yates shuffle.
	srand(time(NULL));
	populate_segment_height(segment_height, SEGMENT_NUMBER);
	shuffle_segments(segment_height, SEGMENT_NUMBER);
	draw_initially(segment_height, SEGMENT_NUMBER);
	sleep(1); //let them breath it in a second

	// RUNNING STATE //
	while (running) 
	{
		// fps handling
		lastFrame = SDL_GetTicks();
		if (lastFrame >= (lastTime+1000)) 
		{
			lastTime = lastFrame;
			fps = frameCount;
			frameCount = 0;
		}

		// look for input
		input();
		
		if(!SORT_COMPLETE) {
			if(SORT == 1) {
				insertionSort(segment_height, SEGMENT_NUMBER);
				draw_finally(segment_height, SEGMENT_NUMBER);
			}
			if (SORT == 2) {			
				quickSort(segment_height, 0, SEGMENT_NUMBER - 1, SEGMENT_NUMBER);
				draw_finally(segment_height, SEGMENT_NUMBER);
			}
			if (SORT == 3) {
				heapSort(segment_height, SEGMENT_NUMBER, SEGMENT_NUMBER);
				draw_finally(segment_height, SEGMENT_NUMBER);
			}
			SORT_COMPLETE = true;
		}
	}

	// clean up
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}
