#pragma once
#include "../Node.hpp"

namespace ms {

class GainNode : public Node {
public:
    GainNode(const std::string& id, float gain = 1.0f)
        : Node(id, 1, 1), gain_(gain) {
        params.push_back({"gain", gain});
    }

    void process(const float* const* inputs, float** outputs, int nFrames) override {
        if (!inputs || !inputs[0] || !outputs || !outputs[0]) return;
        for (int i = 0; i < nFrames; ++i) {
            outputs[0][i] = inputs[0][i] * gain_;
        }
    }

    void setGain(float gain) {
        gain_ = gain;
        params[0].value = gain;
    }

    float getGain() const { return gain_; }

private:
    float gain_;
};

} // namespace ms
