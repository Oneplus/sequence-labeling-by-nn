#ifndef LAYERS_H_INCLUDE
#define LAYERS_H_INCLUDE

#include <vector>

#include "cnn/nodes.h"
#include "cnn/cnn.h"
#include "cnn/training.h"
#include "cnn/rnn.h"
#include "cnn/lstm.h"
#include "cnn/dict.h"
#include "cnn/expr.h"

namespace slnn {

struct BILSTMLayer
{
    cnn::LSTMBuilder *l2r_builder;
    cnn::LSTMBuilder *r2l_builder;
    cnn::Parameters *SOS;
    cnn::Parameters *EOS;
    cnn::expr::Expression SOS_EXP;
    cnn::expr::Expression EOS_EXP;

    BILSTMLayer(cnn::Model *model , unsigned nr_lstm_stack_layers, unsigned lstm_x_dim, unsigned lstm_h_dim);
    ~BILSTMLayer();
    void new_graph(cnn::ComputationGraph &cg);
    void start_new_sequence();
    void build_graph(const std::vector<cnn::expr::Expression> &X_seq , std::vector<cnn::expr::Expression> &l2r_outputs , 
        std::vector<cnn::expr::Expression> &r2l_outputs);

};

struct DenseLayer
{
    cnn::Parameters *w,
        *b;
    cnn::expr::Expression w_exp,
        b_exp;
    DenseLayer(cnn::Model *m , unsigned input_dim , unsigned output_dim );
    ~DenseLayer();
    inline void new_graph(cnn::ComputationGraph &cg);
    inline Expression build_graph(cnn::expr::Expression &e);
};

struct Merge2Layer
{
    cnn::Parameters *w1,
        *w2,
        *b;
    cnn::expr::Expression w1_exp,
        w2_exp,
        b_exp;
    Merge2Layer(cnn::Model *model , unsigned input1_dim, unsigned input2_dim, unsigned output_dim );
    ~Merge2Layer();
    inline void new_graph(cnn::ComputationGraph &cg);
    inline cnn::expr::Expression build_graph(cnn::expr::Expression &e1, cnn::expr::Expression &e2);
};

struct Merge3Layer
{
    cnn::Parameters *w1 ,
        *w2 , 
        *w3 ,
        *b;
    cnn::expr::Expression w1_exp,
        w2_exp,
        w3_exp,
        b_exp;
    Merge3Layer(cnn::Model *model ,unsigned input1_dim , unsigned input2_dim , unsigned input3_dim , unsigned output_dim);
    ~Merge3Layer();
    inline void new_graph(cnn::ComputationGraph &cg);
    inline Expression build_graph(cnn::expr::Expression &e1, cnn::expr::Expression &e2, cnn::expr::Expression &e3);
};




// ------------------- inline function definition --------------------

inline
void BILSTMLayer::new_graph(cnn::ComputationGraph &cg)
{
    l2r_builder->new_graph(cg);
    r2l_builder->new_graph(cg);
    SOS_EXP = parameter(cg, SOS);
    EOS_EXP = parameter(cg, EOS);
}

inline
void BILSTMLayer::start_new_sequence()
{
    l2r_builder->start_new_sequence();
    r2l_builder->start_new_sequence();
}

inline
void BILSTMLayer::build_graph(const std::vector<cnn::expr::Expression> &X_seq, std::vector<cnn::expr::Expression> &l2r_outputs,
    std::vector<cnn::expr::Expression> &r2l_outputs)
{
    size_t seq_len = X_seq.size();
    std::vector<cnn::expr::Expression> tmp_l2r_outputs(seq_len),
        tmp_r2l_outputs(seq_len);
    l2r_builder->add_input(SOS_EXP);
    r2l_builder->add_input(EOS_EXP);
    for (int pos = 0; pos < static_cast<int>(seq_len); ++pos)
    {
        tmp_l2r_outputs[pos] = l2r_builder->add_input(X_seq[pos]);
        int reverse_pos = seq_len - pos - 1;
        tmp_r2l_outputs[reverse_pos] = r2l_builder->add_input(X_seq[reverse_pos]);
    }
    swap(l2r_outputs, tmp_l2r_outputs);
    swap(r2l_outputs, tmp_r2l_outputs);
}

// DenseLayer
inline 
void DenseLayer::new_graph(cnn::ComputationGraph &cg)
{
    w_exp = parameter(cg, w);
    b_exp = parameter(cg, b);
}
inline
Expression DenseLayer::build_graph(cnn::expr::Expression &e)
{
    return affine_transform({ 
       b_exp ,
       w_exp , e 
    });
}

// Merge2Layer 
inline 
void Merge2Layer::new_graph(cnn::ComputationGraph &cg)
{
    b_exp = parameter(cg, b);
    w1_exp = parameter(cg, w1);
    w2_exp = parameter(cg, w2);
}
inline
cnn::expr::Expression Merge2Layer::build_graph(cnn::expr::Expression &e1, cnn::expr::Expression &e2)
{
    return affine_transform({
        b_exp ,
        w1_exp , e1,
        w2_exp , e2,
    });
}

// Merge3Layer
inline 
void Merge3Layer::new_graph(cnn::ComputationGraph &cg)
{
    b_exp = parameter(cg, b);
    w1_exp = parameter(cg, w1);
    w2_exp = parameter(cg, w2);
    w3_exp = parameter(cg, w3);

}
inline
cnn::expr::Expression Merge3Layer::build_graph(cnn::expr::Expression &e1, cnn::expr::Expression &e2, cnn::expr::Expression &e3)
{
    return affine_transform({
        b_exp,
        w1_exp, e1 ,
        w2_exp, e2 ,
        w3_exp, e3
    });
}

}


#endif
