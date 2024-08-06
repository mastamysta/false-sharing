# Deoptomisation

This repo is really just an exercise in de-optimisation (in the interest of learning and understanding relative scales of cache-effects).

# Cache decoherence

Initially I set up a program which increments a set of counters 100000 times. Each counter sits on a separate cache line. The program may take one of two approaches to incrementing these counters:

Increment each counter all the way from 0 to 100000 before moving on:
```
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        for (int j = 0; j < TGT_CNT; j++)
        {
            data[i].a++;
        }
    }
```

Increment each counter once before moving on to the next, looping 100000 times:
```
    for (int j = 0; j < TGT_CNT; j++)
    {
        for (int i = 0; i < ARRAY_SIZE; i++)
        {
            data[i].a++;
        }
    }
```

Both programs perform the same number of increments on counters and produce very similar object code:

Non-Striding:
```
    113e:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
    1145:	eb 27                	jmp    116e <single_threaded()+0x45>
    1147:	8b 45 f8             	mov    -0x8(%rbp),%eax
    114a:	48 98                	cltq
    114c:	48 89 c6             	mov    %rax,%rsi
    114f:	48 8d 05 ea 2e 00 00 	lea    0x2eea(%rip),%rax        # 4040 <data>
    1156:	48 89 c7             	mov    %rax,%rdi
    1159:	e8 d6 00 00 00       	call   1234 <std::array<cache_line, 100000ul>::operator[](unsigned long)>
    115e:	48 8b 50 38          	mov    0x38(%rax),%rdx
    1162:	48 83 c2 01          	add    $0x1,%rdx
    1166:	48 89 50 38          	mov    %rdx,0x38(%rax)
    116a:	83 45 fc 01          	addl   $0x1,-0x4(%rbp)
    116e:	81 7d fc 9f 86 01 00 	cmpl   $0x1869f,-0x4(%rbp)
    1175:	7e d0                	jle    1147 <single_threaded()+0x1e>
    1177:	83 45 f8 01          	addl   $0x1,-0x8(%rbp)
    117b:	81 7d f8 9f 86 01 00 	cmpl   $0x1869f,-0x8(%rbp)
    1182:	7e ba                	jle    113e <single_threaded()+0x15>
```

Striding:
```
    119d:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
    11a4:	eb 27                	jmp    11cd <single_threaded_striding()+0x45>
    11a6:	8b 45 fc             	mov    -0x4(%rbp),%eax
    11a9:	48 98                	cltq
    11ab:	48 89 c6             	mov    %rax,%rsi
    11ae:	48 8d 05 8b 2e 00 00 	lea    0x2e8b(%rip),%rax        # 4040 <data>
    11b5:	48 89 c7             	mov    %rax,%rdi
    11b8:	e8 77 00 00 00       	call   1234 <std::array<cache_line, 100000ul>::operator[](unsigned long)>
    11bd:	48 8b 50 38          	mov    0x38(%rax),%rdx
    11c1:	48 83 c2 01          	add    $0x1,%rdx
    11c5:	48 89 50 38          	mov    %rdx,0x38(%rax)
    11c9:	83 45 fc 01          	addl   $0x1,-0x4(%rbp)
    11cd:	81 7d fc 9f 86 01 00 	cmpl   $0x1869f,-0x4(%rbp)
    11d4:	7e d0                	jle    11a6 <single_threaded_striding()+0x1e>
    11d6:	83 45 f8 01          	addl   $0x1,-0x8(%rbp)
    11da:	81 7d f8 9f 86 01 00 	cmpl   $0x1869f,-0x8(%rbp)
    11e1:	7e ba                	jle    119d <single_threaded_striding()+0x15>
```

Seriously, properly look at that assembly... those exerpts are output from g++ 13.2.0 and they contain *exactly identical* sequences of instructions just operating on different addresses.

But when we run these virtually identical programs in perf we get dramatic results

Non-Striding:
```
         22,361.69 msec task-clock                       #    1.000 CPUs utilized             
                76      context-switches                 #    3.399 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
             1,614      page-faults                      #   72.177 /sec                      
    85,994,025,250      cycles                           #    3.846 GHz                         (71.43%)
        48,203,603      stalled-cycles-frontend          #    0.06% frontend cycles idle        (71.43%)
   240,324,843,035      instructions                     #    2.79  insn per cycle            
                                                  #    0.00  stalled cycles per insn     (71.43%)
    30,041,413,084      branches                         #    1.343 G/sec                       (71.43%)
           800,328      branch-misses                    #    0.00% of all branches             (71.43%)
   130,071,163,629      L1-dcache-loads                  #    5.817 G/sec                       (71.43%)
         2,323,351      L1-dcache-load-misses            #    0.00% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      22.363013237 seconds time elapsed

```

Striding:
```
         69,344.55 msec task-clock                       #    1.000 CPUs utilized             
               282      context-switches                 #    4.067 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
             1,615      page-faults                      #   23.290 /sec                      
   278,931,471,334      cycles                           #    4.022 GHz                         (71.43%)
     3,035,568,760      stalled-cycles-frontend          #    1.09% frontend cycles idle        (71.43%)
   240,670,238,499      instructions                     #    0.86  insn per cycle            
                                                  #    0.01  stalled cycles per insn     (71.43%)
    30,124,556,701      branches                         #  434.419 M/sec                       (71.43%)
         2,133,509      branch-misses                    #    0.01% of all branches             (71.43%)
   130,701,761,440      L1-dcache-loads                  #    1.885 G/sec                       (71.43%)
    10,026,589,351      L1-dcache-load-misses            #    7.67% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      69.349058765 seconds time elapsed

      69.339328000 seconds user
       0.003999000 seconds sys
```

Notably, the code which strides over each cache line multiple times incurs ~7.5% L1 cache misses and takes *more than three times* as long to execute, despite running at roughly the same clock speed and with roughly the same number of micro-ops.

