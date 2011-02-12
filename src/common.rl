%%{
    machine common;

    action read_char {
        dd("reading %c", *p);
    }

    CRLF = "\r\n"; # $read_char;

    action finalize {
        dd("done!");
        done = 1;
    }

    action read_size {
        ctx->chunk_size *= 10;
        ctx->chunk_size += *p - '0';
        dd("read chunk size: %d", (int) ctx->chunk_size);
    }

    action start_reading_size {
        dd("start reading chunk size");
        ctx->chunk_bytes_read = 0;
        ctx->chunk_size = 0;
    }

    action start_reading_data {
        dd("start reading data");
        ctx->chunk_bytes_read = 0;
    }

    action test_len {
#if 0
        fprintf(stderr, "test chunk len: %d < %d\n",
            (int) ctx->chunk_bytes_read, (int) ctx->chunk_size),
#endif
        ctx->chunk_bytes_read++ < ctx->chunk_size
    }

    chunk_size = ([1-9] digit*) >start_reading_size $read_size
               ;

    chunk_data_octet = any when test_len
                     ;

    chunk_data = chunk_data_octet+;

    action read_chunk {
        ctx->chunks_read++;
        dd("have read chunk %d, %.*s", (int) ctx->chunks_read,
            (int) (p - (char *) b->last), (char *) b->last);
    }

    trailer = CRLF @read_chunk
            ;

    chunk = "$" "0"+ CRLF trailer
          | "$-" digit+ trailer
          | "$" chunk_size CRLF chunk_data trailer
          ;

}%%

