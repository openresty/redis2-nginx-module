%%{
    machine single_line_reply;

    include common "common.rl";

    response = (any* -- CRLF) CRLF
             ;

    main := response @finalize
         ;

}%%

