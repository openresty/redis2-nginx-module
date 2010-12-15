%%{
    machine bulk_reply;

    include common "common.rl";

    response = chunk
             ;

    main := response @finalize
         ;

}%%

