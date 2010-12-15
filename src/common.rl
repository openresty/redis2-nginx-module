%%{
    machine common;

    CRLF = "\r\n";

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
        ctx->chunk_size = 0;
    }

    action start_reading_data {
        ctx->chunk_bytes_read = 0;
    }

    action test_len {
        ctx->chunk_bytes_read++ < ctx->chunk_size
    }

    chunk_size = (digit+ -- "0"+) >start_reading_size $read_size
               ;

    chunk_data_octet = any when test_len
                     ;

    chunk_data = chunk_data_octet+
                 >start_reading_data
               ;

    action read_chunk {
        ctx->chunks_read++;
        dd("read chunk %d", (int) ctx->chunks_read);
    }

    trailer = CRLF @read_chunk
            ;

    chunk = "$" "0"+ CRLF trailer
          | "$-" (digit+ -- "0"+) trailer
          | "$" chunk_size CRLF chunk_data trailer
          ;

}%%

