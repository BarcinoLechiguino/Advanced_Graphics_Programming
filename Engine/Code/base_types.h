#ifndef __BASE_TYPES_H__
#define __BASE_TYPES_H__

typedef char					i8;
typedef short					i16;
typedef int						i32;
typedef long long int			i64;
typedef unsigned char			u8;
typedef unsigned short			u16;
typedef unsigned int			u32;
typedef unsigned long long int	u64;
typedef float					f32;
typedef double					f64;

struct String
{
    char* str;
    u32   len;
};

#endif // !__BASE_TYPES_H__