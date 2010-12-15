%%{
    machine multi_bulk_reply;

    include common "common.rl";

    action test_chunk_count {
        ctx->chunks_read < ctx->chunk_count
    }

    action start_reading_chunk {
        ctx->chunks_read = 0;
    }

    action start_reading_count {
        ctx->chunk_count = 0;
    }

    action read_count {
        ctx->chunk_count *= 10;
        ctx->chunk_count += *p - '0';
        dd("chunk count: %d", (int) ctx->chunk_count);
    }

    protected_chunk = chunk when test_chunk_count
                    ;

    chunk_count = (digit+ -- "0"+) >start_reading_count $read_count
                ;

    response = "*" chunk_count CRLF
               (protected_chunk+ >start_reading_chunk)
             ;

    action multi_bulk_finalize {
        dd("done!");
        if (ctx->chunks_read == ctx->chunk_count) {
            done = 1;
        }
    }


    main := response @multi_bulk_finalize
         ;

}%%

