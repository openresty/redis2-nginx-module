%%{
    machine common;

    CRLF = "\r\n";

    action finalize {
        done = 1;
    }

    action read_size {
        ctx->chunk_size *= 10;
        ctx->chunk_size += *p - '0';
    }

    action start_reading_size {
        ctx->chunk_size = 0;
    }

    action start_reading_data {
        ctx->chunk_bytes_read = 0;
    }

    action read_data_byte {
        ctx->chunk_bytes_read++;
    }

    action test_len {
        ctx->chunk_bytes_read < ctx->chunk_size
    }

}%%

