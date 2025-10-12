#pragma once
#include "../Node.hpp"
#include <cmath>

namespace ms {

class SineNode : public Node {
public:
    SineNode(const std::string& id, float frequency = 440.0f)
        : Node(id, 0, 1), frequency_(frequency), phase_(0.0f) {
        params.push_back({"frequency", frequency});
    }

    void prepare(int sampleRate, int blockSize) override {
        Node::prepare(sampleRate, blockSize);
        phase_ = 0.0f;
        phaseIncrement_ = 2.0f * M_PI * frequency_ / static_cast<float>(sampleRate);
    }

    void process(const float* const* inputs, float** outputs, int nFrames) override {
        if (!outputs || !outputs[0]) return;

        for (int i = 0; i < nFrames; ++i) {
            outputs[0][i] = std::sin(phase_);
            phase_ += phaseIncrement_;
            
                        // Wrap phase to avoid precision issues
                        if (phase_ >= 2.0f * M_PI) {
                            phase_ -= 2.0f * M_PI;
                        }
        }
    }

    void setFrequency(float freq) {
        frequency_ = freq;
        phaseIncrement_ = 2.0f * M_PI * frequency_ / static_cast<float>(sampleRate_);
        params[0].value = freq;
    }

private:
    float frequency_;
    float phase_;
    float phaseIncrement_;
};

} // namespace ms
