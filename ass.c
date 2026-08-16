#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum REGS
{
    REGS_NONE,
    REGS_X,
    REGS_XY
} REGS;

typedef enum VALUE_TYPE
{
    VALUE_NONE,
    VALUE_N,
    VALUE_NN,
    VALUE_NNN
} VALUE_TYPE;

// Parameters (X/Y) will have to go somewhere too..
typedef struct INSTR
{
    const char *mnem;
    const char *hex;
    REGS regs;
    VALUE_TYPE val_type;
} INSTR;

static const INSTR mnems[] = {
    {.mnem = "CLS", .hex = "\x00\xE0", .regs = REGS_NONE, .val_type = VALUE_NONE},
};

int main(void)
{
    FILE *f = fopen("./test.asm", "r");

    char buf[512];

    while (fgets(buf, sizeof(buf), f) != NULL)
    {
        size_t i = 0;

        while (buf[i] == ' ' || buf[i] == '\t')
        {
            i++;
        }

        // Skip lines which are just newlines, empty or purely comments
        if (buf[i] == '\n' || buf[i] == '\0' || buf[i] == ';')
        {
            continue;
        }

        char mnem[64];
        size_t j = 0;

        while (buf[i] != ' ' && buf[i] != '\0' && buf[i] != '\t' && buf[i] != '\n' && j < sizeof(mnem) - 1 && i < sizeof(buf) - 1)
        {
            mnem[j] = buf[i];
            i++;
            j++;
        }

        mnem[j] = '\0';

        bool match = false;

        for (size_t k = 0; k < sizeof mnems / sizeof mnems[0]; k++)
        {
            if (strcmp(mnems[k].mnem, mnem) == 0)
            {
                fprintf(stderr, "Found matching mnemonic: %s\n", mnem);
                match = true;
                break;
            }
        }

        if (!match)
        {
            fprintf(stderr, "Unknown mnemonic: %s", mnem);

            // TODO: Decide whether to break or continue here
            // TODO2: Return line number
        }
    }

    fclose(f);

    return 0;
}
