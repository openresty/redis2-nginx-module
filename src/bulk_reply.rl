%%{
    machine bulk_reply;

    include common "common.rl";

    chunk_size = (digit+ -- "0"+) >start_reading_size $read_size
               ;

    chunk_data_octet = any when test_len
                     ;

    chunk_data = chunk_data_octet+
                 >start_reading_data
                 $read_data_byte
               ;

    chunk = "$0"+ CRLF CRLF
          | "$-" (digit+ - "0"+) CRLF
          | "$" chunk_size CRLF chunk_data CRLF
          ;

    response = chunk
             ;

    main := response @finalize
         ;

}%%

