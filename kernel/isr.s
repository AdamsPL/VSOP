.extern eoi
.extern irq_common

.global _leave_kernel
.global _isr_null

.macro ISR_NEC id
	.global _isr\id
	_isr\id:
		cli
		push $0
		push $\id
		jmp isr_common
.endm

.macro ISR_EC id
	.global _isr\id
	_isr\id:
		cli
		push $\id
		jmp isr_common
.endm


_isr_null:
	iret

isr_common:
	pusha
	xorl %eax, %eax
	mov %ds, %ax
	push %eax

	movl $0xFEE00080, %eax
	movl (%eax), %eax
	push %eax

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	call irq_handler

	cli

	pop %eax
	movl $0xFEE00080, %ebx
	movl %eax, (%ebx)

	pop %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	popa
	add $8, %esp
	iret

_leave_kernel:
	pop %eax
	pop %eax
	pop %eax

	pop %eax
	movl $0xFEE00080, %ebx
	movl %eax, (%ebx)

	pop %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	popa
	add $8, %esp
	iret

ISR_NEC 0
ISR_NEC 1
ISR_NEC 2
ISR_NEC 3
ISR_NEC 4
ISR_NEC 5
ISR_NEC 6
ISR_NEC 7
ISR_EC 8
ISR_NEC 9
ISR_EC 10
ISR_EC 11
ISR_EC 12
ISR_EC 13
ISR_EC 14
ISR_NEC 15
ISR_NEC 16
ISR_NEC 17
ISR_NEC 18
ISR_NEC 19
ISR_NEC 20
ISR_NEC 21
ISR_NEC 22
ISR_NEC 23
ISR_NEC 24
ISR_NEC 25
ISR_NEC 26
ISR_NEC 27
ISR_NEC 28
ISR_NEC 29
ISR_NEC 30
ISR_NEC 31
ISR_NEC 32
ISR_NEC 33
ISR_NEC 34
ISR_NEC 35
ISR_NEC 36
ISR_NEC 37
ISR_NEC 38
ISR_NEC 39
ISR_NEC 40
ISR_NEC 41
ISR_NEC 42
ISR_NEC 43
ISR_NEC 44
ISR_NEC 45
ISR_NEC 46
ISR_NEC 47
ISR_NEC 48
ISR_NEC 49
ISR_NEC 50
ISR_NEC 51
ISR_NEC 52
ISR_NEC 53
ISR_NEC 54
ISR_NEC 55
ISR_NEC 56
ISR_NEC 57
ISR_NEC 58
ISR_NEC 59
ISR_NEC 60
ISR_NEC 61
ISR_NEC 62
ISR_NEC 63
ISR_NEC 64
ISR_NEC 65
ISR_NEC 66
ISR_NEC 67
ISR_NEC 68
ISR_NEC 69
ISR_NEC 70
ISR_NEC 71
ISR_NEC 72
ISR_NEC 73
ISR_NEC 74
ISR_NEC 75
ISR_NEC 76
ISR_NEC 77
ISR_NEC 78
ISR_NEC 79
ISR_NEC 80
ISR_NEC 81
ISR_NEC 82
ISR_NEC 83
ISR_NEC 84
ISR_NEC 85
ISR_NEC 86
ISR_NEC 87
ISR_NEC 88
ISR_NEC 89
ISR_NEC 90
ISR_NEC 91
ISR_NEC 92
ISR_NEC 93
ISR_NEC 94
ISR_NEC 95
ISR_NEC 96
ISR_NEC 97
ISR_NEC 98
ISR_NEC 99
ISR_NEC 100
ISR_NEC 101
ISR_NEC 102
ISR_NEC 103
ISR_NEC 104
ISR_NEC 105
ISR_NEC 106
ISR_NEC 107
ISR_NEC 108
ISR_NEC 109
ISR_NEC 110
ISR_NEC 111
ISR_NEC 112
ISR_NEC 113
ISR_NEC 114
ISR_NEC 115
ISR_NEC 116
ISR_NEC 117
ISR_NEC 118
ISR_NEC 119
ISR_NEC 120
ISR_NEC 121
ISR_NEC 122
ISR_NEC 123
ISR_NEC 124
ISR_NEC 125
ISR_NEC 126
ISR_NEC 127
ISR_NEC 128
ISR_NEC 129
ISR_NEC 130
ISR_NEC 131
ISR_NEC 132
ISR_NEC 133
ISR_NEC 134
ISR_NEC 135
ISR_NEC 136
ISR_NEC 137
ISR_NEC 138
ISR_NEC 139
ISR_NEC 140
ISR_NEC 141
ISR_NEC 142
ISR_NEC 143
ISR_NEC 144
ISR_NEC 145
ISR_NEC 146
ISR_NEC 147
ISR_NEC 148
ISR_NEC 149
ISR_NEC 150
ISR_NEC 151
ISR_NEC 152
ISR_NEC 153
ISR_NEC 154
ISR_NEC 155
ISR_NEC 156
ISR_NEC 157
ISR_NEC 158
ISR_NEC 159
ISR_NEC 160
ISR_NEC 161
ISR_NEC 162
ISR_NEC 163
ISR_NEC 164
ISR_NEC 165
ISR_NEC 166
ISR_NEC 167
ISR_NEC 168
ISR_NEC 169
ISR_NEC 170
ISR_NEC 171
ISR_NEC 172
ISR_NEC 173
ISR_NEC 174
ISR_NEC 175
ISR_NEC 176
ISR_NEC 177
ISR_NEC 178
ISR_NEC 179
ISR_NEC 180
ISR_NEC 181
ISR_NEC 182
ISR_NEC 183
ISR_NEC 184
ISR_NEC 185
ISR_NEC 186
ISR_NEC 187
ISR_NEC 188
ISR_NEC 189
ISR_NEC 190
ISR_NEC 191
ISR_NEC 192
ISR_NEC 193
ISR_NEC 194
ISR_NEC 195
ISR_NEC 196
ISR_NEC 197
ISR_NEC 198
ISR_NEC 199
ISR_NEC 200
ISR_NEC 201
ISR_NEC 202
ISR_NEC 203
ISR_NEC 204
ISR_NEC 205
ISR_NEC 206
ISR_NEC 207
ISR_NEC 208
ISR_NEC 209
ISR_NEC 210
ISR_NEC 211
ISR_NEC 212
ISR_NEC 213
ISR_NEC 214
ISR_NEC 215
ISR_NEC 216
ISR_NEC 217
ISR_NEC 218
ISR_NEC 219
ISR_NEC 220
ISR_NEC 221
ISR_NEC 222
ISR_NEC 223
ISR_NEC 224
ISR_NEC 225
ISR_NEC 226
ISR_NEC 227
ISR_NEC 228
ISR_NEC 229
ISR_NEC 230
ISR_NEC 231
ISR_NEC 232
ISR_NEC 233
ISR_NEC 234
ISR_NEC 235
ISR_NEC 236
ISR_NEC 237
ISR_NEC 238
ISR_NEC 239
ISR_NEC 240
ISR_NEC 241
ISR_NEC 242
ISR_NEC 243
ISR_NEC 244
ISR_NEC 245
ISR_NEC 246
ISR_NEC 247
ISR_NEC 248
ISR_NEC 249

