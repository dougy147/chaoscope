#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

float compute_entropy(unsigned char block[], int block_size) {
	float entropy = 0;
	int max_byte_val = 256;
	int in_block[256] = {0};

	for (int i = 0; i < block_size; i++) {
		int dec = (unsigned char)block[i];
		in_block[dec] += 1;
	}

	for (int i = 0; i < max_byte_val; i ++) {
		double px = (float)in_block[i] / (double)block_size;
		if ( px > 0 ) {
			entropy -= px * log2f(px);
		}
	}
	return ( entropy / 8 ); // 1byte = 8bits
}

int main(int argc, char *argv[]) {
    bool arg_is_file = false;
    FILE *file = NULL;
    unsigned long int file_size; // used only if file passed as first arg

    int block = 0;
    unsigned long int block_size = 256;

    // if a file is passed read it, else read from standard input
    if (argc > 1) {
    	/* file from command line (first arg for the moment) */
    	arg_is_file = true;
    	file = fopen(argv[1], "rb");
    	/* compute file size */
    	fseek(file, 0, SEEK_END);
    	file_size = ftell(file);
    	fseek(file, 0, SEEK_SET);

	/* compute blokc */
        unsigned long int data_points = 2 * block_size;
        block_size = (file_size / data_points) + ((block_size - (file_size / data_points)) % block_size);
        if (block_size <= 0) { block_size = file_size; }

    } else {
    	file = stdin;
    }

    /* Window parameters*/
    int w = 800;
    int h = 600;

    /* graduate scale */
    int h_offset = 0.12*h;
    int h_graph  = h - (2 * h_offset);
    int w_offset = 0.04*w;
    int w_graph  = w - (2 * w_offset);
    int bars = 0.01 * w; // graduation bars

    InitWindow(w, h, "Chaoscope");
    //SetTargetFPS(60);

    /*entropy buffer*/
    float entropy_arr[] = {-1.0};

    int dot_radius = 3;
    int dot_padding = 1;
    int cell_size = dot_radius * 2 + 2 * 0.5 * dot_padding;
    int dot_by_width = w_graph / cell_size;

    if (arg_is_file) {
	int chunks = file_size / block_size;
        cell_size = w_graph / chunks;
	if (cell_size == 0) {
	    /*readjusting block size (too small window)*/
	    block_size = file_size / w_graph;
	    cell_size = 1;
	}
        dot_by_width = w_graph / cell_size ;
    }

    bool reached_border = false; // start moving when reached border
    bool now_move = false; // start moving when reached border
    int  remaining_size = file_size;

    bool end_file = false;

    float entropy;

    unsigned char buffer[block_size]; // TODO: if file too big: fail with grace


    while (!WindowShouldClose()) {

	if (arg_is_file && remaining_size <= 0) {
	    end_file = true;
	    now_move = false;
	}

    	if (arg_is_file && (long unsigned int)remaining_size < block_size && !end_file) { block_size = remaining_size; }

    	if (!end_file) {
	    fread(buffer, 1, block_size, file);
	    entropy = compute_entropy(buffer, block_size);
	    if (now_move) {
	        for (int i = 0; i < dot_by_width - 1; i++) {
	            entropy_arr[i] = entropy_arr[i+1];
	        }
	        entropy_arr[dot_by_width - 1] = entropy;
	    }
	    else {
	        entropy_arr[block] = entropy;
	    }
	    if (!reached_border && block >= dot_by_width) {
	        reached_border = true;
		if (!arg_is_file) {
		    now_move = true; // never move if arg is a file
		} else {
		    end_file = true;
		}
	    }
    	    remaining_size -= block_size;
	}

    	BeginDrawing();
    	ClearBackground(BLACK);

	/*draw scale*/
	DrawLine(w_offset, h_offset ,w_offset, h_offset + h_graph, GRAY);  		      // left vertical line
	DrawLine(w_offset + w_graph, h_offset ,w_offset + w_graph, h_offset + h_graph, GRAY); // right vertical line
	for (int i = 0; i <= 10; i++) {
	    DrawLine(w_offset - (bars * 0.5),
		     (h_offset + h_graph) - (i * h_graph / 10),
		     w_offset + (bars * 0.5),
		     (h_offset + h_graph) - (i * h_graph / 10),
		     GRAY); // small horiz lines (left bar)
	    DrawLine((w_offset + w_graph) - (bars * 0.5),
		     (h_offset + h_graph) - (i * h_graph / 10),
		     (w_offset + w_graph) + (bars * 0.5),
		     (h_offset + h_graph) - (i * h_graph / 10),
		     GRAY); // small horiz lines (right bar)

	    // draw a dotted line from left to right
	    int size_dot = w_graph / 100;
	    for ( int j = 0; j < (w_graph / size_dot); j+=2 ) {
		DrawLine(w_offset + (j * size_dot),
			 (h_offset + h_graph) - (i * h_graph / 10),
			 w_offset + (j * size_dot) + size_dot,
			 (h_offset + h_graph) - (i * h_graph / 10),
			 DARKGRAY);
	    }
	}

	/* current entropy (value and circle) */
	if (!end_file) {
	    Color color = CLITERAL(Color){ 253, 255 * ( 1 - entropy), 0, 255 };
	    DrawText(TextFormat("%0.3f", entropy), 0.1*w, 0.05*h, 30, color);
	    DrawCircle(0.05*w, 0.075*h, 3.0 * entropy * 10, CLITERAL(Color){ 253, 255 * (1 - entropy), 0, 255 * entropy });
	} else {
	    char *filename[500]; // if i don't declare this, it fails... why?
	    int font_size = 20;
	    DrawText(TextFormat("%s", argv[1]), 0.04*w, 0.04*h, font_size, YELLOW);
	}

	for (int i = 0; i < block; i++) {
	    if (entropy_arr[i] == -1 ) continue;

	    if (i + 1 < block - 1) {
	        Color color = CLITERAL(Color){ 253, 255 * ( 1 - entropy_arr[i+1]), 0, 255 };
	        DrawLine(w_offset  + (i * cell_size),
			 h_offset + h_graph - (entropy_arr[i] * h_graph),
			 w_offset + ((i + 1) * cell_size),
			 h_offset + h_graph - (entropy_arr[i+1] * h_graph),
			 color);
	    }
	}

	DrawText("chaoscope", w/2 - (strlen("chaoscope")/2 ) * cell_size, h - h_offset + (h_offset / 3), 20, DARKGRAY);

    	EndDrawing();

	if (!now_move && !end_file) { block++; }
    }

    CloseWindow();
    return 0;
}
