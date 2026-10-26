#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    //(void)challenge;
    // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);

    uint32_t step2 = (high + low * 8u) ^ 0xE76Cu;
    uint32_t step0 = (low * 3u + mid) ^ 0x13B3u;

    values[0] = (step2 + 0x693Du) ^ step0;
    count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_02 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    values[0] = (uint32_t)(((challenge >> 20) & 0x7Fu) ^ 0x0Fu);

    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t nibble = (uint32_t)((challenge >> (i + 11u)) & 0x0Fu);
        values[i] = (nibble + values[i - 1]) ^ (3u * i);
    }

    count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    //(void)challenge;
    // TODO: 根据 phase_03 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t selector = (uint32_t)(challenge & 7u);
    uint32_t mixed = (uint32_t)(challenge >> 32)
                   ^ (uint32_t)(challenge >> 3);

    values[0] = selector;

    switch (selector) {
        case 0:
            values[1] = mixed * 3u + 0xD2CAu;
            break;
        case 1:
            values[1] = mixed * 5u + 0xBD56u;
            break;
        case 2:
            values[1] = (mixed * 4u) ^ 0x6127u;
            break;
        case 3:
            values[1] = mixed * 5u + 0x6787u;
            break;
        case 4:
            values[1] = mixed * 5u + 0x22844u;
            break;
        case 5:
            values[1] = (mixed * 2u) ^ 0x2BF1u;
            break;
        case 6:
            values[1] = (mixed ^ 0x1A79u) + (mixed >> 1);
            break;
        case 7:
            values[1] = (mixed ^ 0x1947u) + (mixed >> 1);
            break;
    }

    count = 2;
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk_local(uint32_t target,
                              uint32_t low,
                              uint32_t high,
                              uint32_t key) {
    uint32_t mid = low + ((high - low) >> 1);

    if (mid == target) {
        return (mid + 0x71u) ^ key;
    }

    uint32_t mixed = mid + key;

    if (target > mid) {
        uint32_t result = p4_walk_local(
            target, mid + 1u, high, key
        );
        return ((result * 3u) ^ mixed) ^ 0x1Au;
    } else {
        uint32_t result = p4_walk_local(
            target, low, mid - 1u, key
        );
        return ((result * 3u) ^ mixed) ^ 0x16u;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_04 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。

    uint32_t target = (uint32_t)(challenge & 0x7Fu);
    uint32_t key = (uint32_t)(challenge >> 7)
                 ^ (uint32_t)(challenge >> 32);

    values[0] = p4_walk_local(target, 0u, 0x7Fu, key);
    count = 1;

    return write_answer(out, cap, values, count);
}

static uint32_t p5_traverse_local(uint64_t challenge, uint32_t *last_a) {
    struct P5Node {
        uint32_t next;
        uint32_t a;
        uint32_t b;
    };

    // 按节点地址从低到高排列，next 表示下一个节点的数组下标。
    static const struct P5Node nodes[12] = {
        {5u,          0x231u,  0xDFCFu}, // 0x405ba0
        {8u,          0x326u,  0x8EEAu}, // 0x405bb0
        {4u,          0x448u,  0xD1BCu}, // 0x405bc0
        {10u,         0x66Au,  0x38BDu}, // 0x405bd0
        {0u,          0xA2Bu,  0x269Cu}, // 0x405be0
        {7u,          0x877u,  0xC2E1u}, // 0x405bf0
        {0xFFFFFFFFu, 0xB66u,  0x63CAu}, // 0x405c00
        {1u,          0x547u,  0x3C82u}, // 0x405c10
        {9u,          0x742u,  0xE26Cu}, // 0x405c20
        {3u,          0x958u,  0xC2ABu}, // 0x405c30
        {11u,         0xDu,    0x159Bu}, // 0x405c40
        {6u,          0x163u,  0x938Fu}  // 0x405c50
    };

    // challenge 低 3 位决定起始节点。
    static const uint32_t starts[8] = {
        2u, 4u, 0u, 5u, 7u, 1u, 8u, 9u
    };

    uint32_t idx = starts[challenge & 7u];
    uint32_t steps = (uint32_t)((challenge >> 3) & 3u);

    uint32_t result =
        ((uint32_t)(challenge >> 6)
         ^ (uint32_t)(challenge >> 32))
        ^ 0x3E6Fu;

    // 先处理起始节点，再沿链表继续遍历 steps 次。
    for (uint32_t i = 0; i <= steps; ++i) {
        result = (result + nodes[idx].b) * 5u;
        result ^= nodes[idx].a;

        *last_a = nodes[idx].a;

        if (i < steps) {
            idx = nodes[idx].next;
        }
    }

    return result;
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    (void)challenge;
    // TODO: 根据 phase_05 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。

    uint32_t last_a = 0;

    values[0] = p5_traverse_local(challenge, &last_a);
    values[1] = last_a;

    count = 2;

    return write_answer(out, cap, values, count);
}

static uint32_t p6_apply_local(uint32_t index, uint32_t x) {
    switch (index) {
        case 0:
            return (x ^ 0x5B5Eu) + x * 2u;

        case 1:
        case 2:
        case 3:
            return ((x << 2) | (x >> 30)) ^ 0x40DEu;

        case 4:
            return (x + 0x2763u) ^ (x >> 7);

        case 5:
            return (x >> 3) | (x << 29);

        case 6:
            return ((x << 5) + 0x49A0u) ^ (x * 3u);

        case 7:
            return ((x >> 2) ^ x) + 0x454Au;

        default:
            return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    (void)challenge;
    // TODO: 根据 phase_06 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。

    uint32_t arg1 = (uint32_t)(challenge >> 6)
                  ^ (uint32_t)(challenge >> 32);

    uint32_t index1 = (uint32_t)(challenge & 7u);
    uint32_t result1 = p6_apply_local(index1, arg1);

    uint32_t arg2 = result1 ^ 0xF593u;
    uint32_t index2 = (uint32_t)((challenge >> 3) & 7u);
    uint32_t result2 = p6_apply_local(index2, arg2);

    values[0] = result2 + 0xE8B7u;
    count = 1;

    return write_answer(out, cap, values, count);
}


int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。

    uint32_t c = (uint32_t)challenge
               ^ (uint32_t)(challenge >> 32);
    uint32_t k = (uint32_t)context
               ^ (uint32_t)(context >> 32);

    // 第一个整数：context 的低两位。
    uint32_t v0 = (uint32_t)context & 3u;

    // secret_mix：32 位循环右移一位，再异或常量。
    uint32_t mix_input = c ^ k;
    uint32_t mixed = ((mix_input >> 1) | (mix_input << 31))
                   ^ 0x6887u;

    // 第二个整数：对应 secret_phase 中的 eax 和 r9d 运算。
    uint32_t r9 = (v0 * 0x951Cu)
                ^ (uint32_t)(context >> 16);
    uint32_t v1 = mixed ^ r9;

    // 第三个整数：注意括号中的加法先完成，再与 v1 异或。
    uint32_t v2 = v1
                ^ (k + v0 + 0xFD9Bu + (c >> 1));

    if (out == NULL || cap == 0) {
        return 1;
    }

    // 前 32 个字符后必须紧接 OPEN，secret_phase 从其后解析三个整数。
    int written = snprintf(
        out, cap,
        "AAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAA"
        "OPEN %" PRIu32 " %" PRIu32 " %" PRIu32,
        v0, v1, v2
    );

    if (written < 0 || (size_t)written >= cap) {
        return 1;
    }

    return 0;
}
