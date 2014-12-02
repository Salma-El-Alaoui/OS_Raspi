
define pirqs
  set $stack_size = 15
  set $i = $stack_size
  while ( $i > 0 )
     printf "%d   Ox%x ", $i, *((int*) 0x8000 - $i)
     if ($i == $stack_size)
     	printf " <-- bp"
     end
     printf  "\n"
     set $i = $i - 1
  end
end

define pstack
  set $stack_pointer = current_ctx->ctx_stack + current_ctx->ctx_size
  set $i = 0
  while ( $stack_pointer > ($sp - 1) )
     printf "%d   Ox%x ", $i, *((int*) $stack_pointer)
     if ($i == $stack_size)
     	printf " <-- bp"
     end
     printf  "\n"
     set $i = $i + 1
     set $stack_pointer = $stack_pointer - 1
  end
end

define psp
       p/x *((int*) $sp)@10
end

define stack_base
       set $height = current_ctx->ctx_stack +  (current_ctx->ctx_size)
       p $height
end

define stack_height
       set $base = current_ctx->ctx_stack + (current_ctx->ctx_size)
       set $height = ((int) $base) - ((int) $sp)
       p $height
end
#{0x0, 0x22900, 0x228e8, 0x227c8, 0x226a8, 0x205b8, 0x1, 0x0, 0x2269c, 0xc350}

#0x0, 0x27de0, 0x27dc8, 0x27ca8, 0x27b88, 0x25a98, 0x1, 0x0, 0x27b7c, 0xc350
