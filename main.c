#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <raylib.h>
#include "resultlist.h"

#define SCREEN_WIDTH	960
#define SCREEN_HEIGHT	600
#define FONT_SIZE 		20
#define BOX_SIZE  		30
#define START_POS_X 	10
#define START_POS_Y 	10
#define PADDING 		4
#define COUNT 		((SCREEN_HEIGHT/BOX_SIZE)-2)
#define COUNT_X		((SCREEN_WIDTH/(FONT_SIZE-START_POS_X))-START_POS_X)

typedef struct
{
	bool is_file_finder;
	char cwd[1023];
	int current_index;
	int total_count;
	int current_x;
}mf_state_t;

mf_state_t *mf_state = NULL;

static void handle_input(void);

static char* get_location(char *string)
{
	char *start =  strchr(string, '/');
	if (NULL == start)
	{
		return NULL;
	}
	char *end = strchr(start, ':');
	if (NULL == end)
	{
		end = strchr(start, ' ');
		if (NULL == end)
		{
			return NULL;
		}
	}

	int length = end - start;
	char *buf = (char*)malloc( (length+1) * sizeof(char));
	strncpy(buf, start, length);
	buf[length] = '\0';
	return buf;
}

static int init_arguments(int argc, char **argv)
{
	
	if (argc < 3)
	{
		return -1;
	}
	
	char* flag = *(argv+1);
	char* search_string = *(argv+2);

	FILE *out;
	char buf[512];	
	char path[1024];

	snprintf(buf, sizeof(buf), "pwd");
	
	out = popen(buf, "r");
	
	if (NULL == out)
	{
		return -1;
	}

	fgets(mf_state->cwd, sizeof(mf_state->cwd), out);
	mf_state->cwd[strlen(mf_state->cwd)-1] = '\0';

	pclose(out);
	
	if (strcmp(flag, "-f") == 0)
	{
		mf_state->is_file_finder = true;
#if WINDOW
		snprintf(buf, sizeof(buf), "dir /b/s *%s*", search_string);
#else
		snprintf(buf, sizeof(buf), "find ./ -name *%s*", search_string);
#endif
	}
	else
	{
		mf_state->is_file_finder = false;
#if WINDOW	
		snprintf(buf, sizeof(buf), "findstr /spin /c:%s ./*.*", search_string);
#else
		snprintf(buf, sizeof(buf), "grep -r %s ./", search_string);
#endif
	}
	
	out = popen(buf, "r");
	
	if (NULL == out)
	{
		return -1;
	}
	int index = 0;
	
	while(fgets(path, sizeof(path), out) != NULL)
	{
		result_t* result  = (result_t*) malloc(sizeof(result_t));
		assert (NULL != result && "Out of memory");
		
		int length = strlen(path);
		
		result->string = (char*) malloc(length * sizeof(char));
		assert (NULL != result->string && "Out of memory");
		strncpy(result->string, path, length);
		result->string[length-1] = '\0';
		char *location = get_location(path);
		if (NULL == location)
		{
			location = ".";
		}
		length = strlen(location);	
		
		if (mf_state->is_file_finder)
		{
#if WINDOW
			result->location = result->string;
#else	
			result->location = (char*) malloc((length+1)*sizeof(char));
			assert (NULL != result->location && "Out of memory");
			strncpy(result->location, location, length);
			result->location[length] = '\0';
#endif
		}
		else
		{
			result->location = (char*) malloc((length+1)*sizeof(char));
			assert (NULL != result->location && "Out of memory");
			strncpy(result->location, location, length);
			result->location[length] = '\0';
		}
			
		result->index = index;	
		index++;
		add_result(result);	
		printf(".");
	}
	pclose(out);
	printf("\n");
	return index;
}

int main(int argc, char** argv)
{
	mf_state = (mf_state_t*)calloc(1, sizeof(mf_state_t));
	assert(NULL != mf_state && "Out of memory");

	mf_state->total_count = init_arguments(argc, argv);
	if (mf_state->total_count == 0)
	{
		printf("No match found\n");
		return mf_state->total_count;
	}else if (mf_state->total_count == -1)
	{
		printf("Error running search commands\n");
		return mf_state->total_count;
	}

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My Finder");
	SetTargetFPS(10);
	
	
	mf_state->current_index = 0;
	mf_state->current_x = 0;
	while (!WindowShouldClose())
	{
		handle_input();
		BeginDrawing();
			ClearBackground(LIGHTGRAY);

			int max_index = mf_state->current_index+COUNT > mf_state->total_count-1 ? mf_state->total_count-1:mf_state->current_index+COUNT;
			
			for (int i = mf_state->current_index; i <= max_index; i++)
			{
				result_t* result = find_result(i);
				char buf[COUNT_X+1];
				memset(buf, '\0', COUNT_X+1);
				if (i == mf_state->current_index)
				{
					DrawRectangle(START_POS_X, START_POS_Y+(BOX_SIZE*(i-mf_state->current_index+1)), SCREEN_WIDTH-START_POS_X*2, BOX_SIZE, WHITE);
					snprintf(buf, COUNT_X, &result->string[mf_state->current_x]);
					DrawText(buf, START_POS_X+PADDING, START_POS_Y+(BOX_SIZE*(i-mf_state->current_index+1))+PADDING, FONT_SIZE, BLACK);
				}
				else
				{
					snprintf(buf, COUNT_X, result->string);
					DrawText(buf, START_POS_X+PADDING, START_POS_Y+(BOX_SIZE*(i-mf_state->current_index+1))+PADDING, FONT_SIZE, BLACK);
				}
			}
		EndDrawing();
	}
	free_resources();
	return 0;
}

static void handle_input()
{
	if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_K))
	{
		if (mf_state->current_index <= 0)
		{
			mf_state->current_index = mf_state->total_count-1;
		}
		else
		{
			mf_state->current_index--;
		}
		mf_state->current_x = 0;
	}
	if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J))
	{
		if (mf_state->current_index >= mf_state->total_count-1)
		{
			mf_state->current_index = 0;
		}
		else
		{
			mf_state->current_index++;
		}
		mf_state->current_x = 0;
	}

	if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_H))
	{
		if (mf_state->current_x <= 0)
		{
			result_t *result = find_result(mf_state->current_index);
			int length = strlen(result->string);
			if (length-COUNT_X > 0)
			{
				mf_state->current_x = length-COUNT_X+1;
			}
		}
		else
		{
			mf_state->current_x--;
		}
	}
	if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_L))
	{
		result_t *result = find_result(mf_state->current_index);
		int length = strlen(result->string);
		if (mf_state->current_x > length-COUNT_X)
		{
			mf_state->current_x = 0;
		}
		else
		{
			mf_state->current_x++;
		}
	}
	if (IsKeyPressed(KEY_M))
	{
		if (mf_state->current_index+COUNT >  mf_state->total_count-1)
		{
			mf_state->current_index = mf_state->total_count-1;
		}
		else
		{
			mf_state->current_index += COUNT;
		}
		mf_state->current_x = 0;
	}

	if (IsKeyPressed(KEY_N))
	{
		if (mf_state->current_index-COUNT < 0)
		{
			mf_state->current_index  = 0;
		}
		else
		{
				mf_state->current_index -= COUNT;
		}
		mf_state->current_x = 0;
	}

	if (IsKeyPressed(KEY_ENTER))
	{
		result_t* result = find_result(mf_state->current_index);
		char buf[2048];
		if (mf_state->is_file_finder)
		{
#if WINDOW
			snprintf(buf, sizeof(buf), "start %s", result->location);
			system(buf);
#else
			snprintf(buf, sizeof(buf), "open %s%s",mf_state->cwd, result->location);
			printf("Path: %s", buf);
			system(buf);
#endif
		}
		else
		{
#if WINDOW
			snprintf(buf, sizeof(buf), "start %s%s",mf_state->cwd, result->location);
#else
			snprintf(buf, sizeof(buf), "open %s%s",mf_state->cwd, result->location);
			printf("Path: %s", buf);
#endif
			system(buf);
		}
	}

}
