#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

// Compress function
int compress_file(FILE *source, FILE *destination) {
    char in_buffer[1024];
    char out_buffer[1024];
    z_stream strm;
    int ret, flush;
    unsigned have;
    
    // Initialize the zlib stream
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    ret = deflateInit(&strm, Z_BEST_COMPRESSION);
    if (ret != Z_OK) return ret;
    
    // Compress the input file
    do {
        strm.avail_in = fread(in_buffer, 1, sizeof(in_buffer), source);
        if (ferror(source)) {
            deflateEnd(&strm);
            return Z_ERRNO;
        }
        
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in_buffer;
        
        do {
            strm.avail_out = sizeof(out_buffer);
            strm.next_out = out_buffer;
            
            ret = deflate(&strm, flush);
            if (ret == Z_STREAM_ERROR) {
                deflateEnd(&strm);
                return ret;
            }
            
            have = sizeof(out_buffer) - strm.avail_out;
            if (fwrite(out_buffer, 1, have, destination) != have || ferror(destination)) {
                deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
    } while (flush != Z_FINISH);
    
    deflateEnd(&strm);
    return Z_OK;
}

// Decompress function
int decompress_file(FILE *source, FILE *destination) {
    char in_buffer[1024];
    char out_buffer[1024];
    z_stream strm;
    int ret;
    unsigned have;
    
    // Initialize the zlib stream
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    ret = inflateInit(&strm);
    if (ret != Z_OK) return ret;
    
    // Decompress the input file
    do {
        strm.avail_in = fread(in_buffer, 1, sizeof(in_buffer), source);
        if (ferror(source)) {
            inflateEnd(&strm);
            return Z_ERRNO;
        }
        
        if (strm.avail_in == 0) break;
        
        strm.next_in = in_buffer;
        
        do {
            strm.avail_out = sizeof(out_buffer);
            strm.next_out = out_buffer;
            
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_ERRNO || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                return ret;
            }
            
            have = sizeof(out_buffer) - strm.avail_out;
            if (fwrite(out_buffer, 1, have, destination) != have || ferror(destination)) {
                inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);
    
    inflateEnd(&strm);
    return Z_OK;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <compress/decompress> <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    FILE *in_file = fopen(argv[2], "rb");
    if (!in_file) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }
    
    FILE *out_file = fopen(argv[3], "wb");
    if (!out_file) {
        perror("Error opening output file");
        fclose(in_file);
        return EXIT_FAILURE;
    }
    
    int ret = 0;
    if (strcmp(argv[1], "compress") == 0) {
        ret = compress_file(in_file, out_file);
        if (ret == Z_OK) {
            printf("Compression successful!\n");
        } else {
            fprintf(stderr, "Compression failed: %d\n", ret);
        }
    } else if (strcmp(argv[1], "decompress") == 0) {
        ret = decompress_file(in_file, out_file);
        if (ret == Z_OK) {
            printf("Decompression successful!\n");
        } else {
            fprintf(stderr, "Decompression failed: %d\n", ret);
        }
    } else {
        fprintf(stderr, "Invalid operation. Use 'compress' or 'decompress'.\n");
        ret = EXIT_FAILURE;
    }
    
    fclose(in_file);
    fclose(out_file);
    
    return ret == Z_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

