%%{
    machine status_code_reply;

    include common "common.rl";

    CRLF = "\r\n";

    response = (any* -- CRLF) CRLF
             ;

    main := response @finalize
         ;

}%%

