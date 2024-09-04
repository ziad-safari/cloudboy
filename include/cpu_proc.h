#include <common.h>
#include <cpu.h>
class CPUProc {
    private:
    CPU& cpu;
    public:
        CPUProc(CPU& cpuref) : cpu(cpuref) {}
        void proc_none();
        void proc_nop();
        void proc_di();
        void proc_load();
        void proc_jp();
        void proc_jr();
        void proc_ldh();
        void proc_xor();
        void proc_call();
        void proc_rst();
        void proc_ret();
        void proc_reti();
        void proc_pop();
        void proc_push();
        using IN_PROC = void (CPUProc::*)();
        IN_PROC inst_get_processor(in_type type);
        bool check_cond();
        void cpu_set_flags(char z, char n, char h, char c);
        void goto_addr(u16 addr, bool pushpc);
        
};