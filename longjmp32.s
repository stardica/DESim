/*
 * longjmp32.s
 *
 *  Created on: Mar 25, 2017
 *      Author: stardica
 */


.section .text
.globl longjmp32_2
.type longjmp32_2, @function
longjmp32_2:
mov    0x4(%esp),%eax
mov    0x14(%eax),%edx
mov    0x10(%eax),%ecx
mov    (%eax),%ebx
mov    0x4(%eax),%esi
mov    0x8(%eax),%edi
mov    0xc(%eax),%ebp
mov    0x8(%esp),%eax
mov    %ecx,%esp
jmp    *%edx


/*
.section .text
.globl longjmp32_2
.type longjmp32_2, @function
longjmp32_2:
mov    0x4(%esp),%eax
mov    0x14(%eax),%edx
mov    0x10(%eax),%ecx
ror    $0x9,%edx
xor    %gs:0x18,%edx
ror    $0x9,%ecx
xor    %gs:0x18,%ecx
mov    (%eax),%ebx
mov    0x4(%eax),%esi
mov    0x8(%eax),%edi
mov    0xc(%eax),%ebp
mov    0x8(%esp),%eax
mov    %ecx,%esp
jmp    *%edx
*/