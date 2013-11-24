        .model  large, c
        .code

        PUBLIC wcompare
        ;wcompare(s1, s2, count)
wcompare PROC far s1: dword, s2: dword, count: word
	push ds
	push si
	push di
	cld

        lds si, s1
        les di, s2
        mov cx, count
	inc cx
	repe cmpsw

        mov ax, count
	sub ax,cx

	pop di
	pop si
	pop ds
        ret
wcompare ENDP

        PUBLIC bcompare
        ;bcompare(s1,s2,count)
;return # of bytes of s1 and s2 that match
bcompare PROC far s1: dword, s2: dword, count: word
	push ds
	push si
	push di
	cld

        lds si, s1
        les di, s2
        mov cx, count
	inc cx
	repe cmpsb

        mov ax, count
	sub ax,cx

	pop di
	pop si
	pop ds
	ret	
bcompare ENDP

        PUBLIC bcontrast
        ;bcontrast(s1, s2, count)
;return how many bytes of s1 and s2 are different
bcontrast PROC  far s1: dword, s2: dword, count: word
        push    ds
        push    si
        push    di
	cld

        lds     si, s1
        les     di, s2
        mov     cx, count
	repne cmpsb
        inc     cx
        mov     ax, count
        sub     ax,cx

        pop     di
        pop     si
        pop     ds
	ret	
bcontrast ENDP

        PUBLIC bsame
        ;bsame(d, count)
	;find out how many bytes in a row are the same value 
bsame   PROC    far d:dword, count: word
        push    di
	cld

        les     di, d
        mov     cx, count
        mov     ax,es:[di]
        inc     cx
	repe scasb

        mov     ax, count
        sub     ax,cx

        pop     di
	ret	
bsame   ENDP

        PUBLIC fii_tnskip, fii_tnsame

;fii_tnskip(s1,s2,bcount,mustmatch)
fii_tnskip     proc far s1: dword, s2: dword, bcount: word, mmatch: word
difcount	equ	[bp-2]
	sub sp,4	;space for locals
	push bx
	push si
	push di
	push ds

	mov word ptr difcount,0	;zero out return value
	lds si,s1
	les di,s2
	mov bx,bcount
	mov dx,mmatch

tnsloop:
	;calculate number of pixels different in s1 and s2 into ax
	mov cx,bx
	inc cx
	repne cmpsb
	mov ax,bx
	sub ax,cx
	dec si	;move source pointers just past this different run
	dec di
	sub bx,ax
	add difcount,ax	;and different count to return value

	cmp bx,dx		;see if near the end...
	js endcheck

	;see if enough in a row match to break out of this
	mov cx,dx
	repe cmpsb
	jz zfii_tnskip	;if all of them match between s1 and s2 go home
	inc cx
	mov ax,dx		;calc ones that do match into ax
	sub ax,cx
	add difcount,ax	;add it to difcount return value
	sub bx,ax		;sub it from pixels left to examine
	dec si		;update s1,s2 pointers
	dec di
	jmp tnsloop
endcheck:
	;check last couple of pixels
	mov cx,bx
	inc cx
	repe cmpsb
	jcxz zfii_tnskip	;if all of them match between s1 and s2 go home
	add difcount,bx	;otherwise assume no skip this time around

zfii_tnskip:
	mov ax,difcount
	pop ds
	pop di
	pop si
	pop bx
	mov sp,bp
	ret
fii_tnskip     endp


;fii_tnsame(s2x,wcount,mustmatch)
fii_tnsame     PROC far s2x: dword, wcount: word, mumatch: word
	push ds
	push si
	push di
	push bx

				
	les	di,s2x		;get starting address in es:di
	mov	dx,wcount		;dx is 'dif_count' return value
	mov	bx,dx		;bx is # of pixels left
	mov	si,0		;si is # of pixels examined
alp:
				;break out of loop if less than 4 pixels
				;left to examine
	cmp	bx,mumatch
	js	zalp

	;same_count = i86_bsame(s2x,wcount)
	mov cx,bx
	mov al,es:[di]
	rep scasb
	inc cx
	sub di,2
	mov ax,bx
	sub ax,cx

	cmp ax,mumatch			;if mustmatch or more
	jns gotsame		;go truncate dif_count
	add	si,ax
	add	di,ax
	sub	bx,ax
	jmp	alp
gotsame:
	mov	dx,si		
zalp:
	mov ax,dx		;return dif_count
	pop bx
	pop di
	pop si
	pop ds
	ret
fii_tnsame     ENDP

END
