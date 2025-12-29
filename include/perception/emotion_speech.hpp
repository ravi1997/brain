#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

namespace dnn::perception {

// Emotion Recognition from Speech using prosodic features
class EmotionRecognitionSpeech {
public:
    enum class Emotion {
        NEUTRAL,
        HAPPY,
        SAD,
        ANGRY,
        FEARFUL,
        SURPRISED,
        DISGUSTED
    };
    
    struct ProsodicFeatures {
        float pitch_mean;
        float pitch_variance;
        float energy_mean;
        float energy_variance;
        float speaking_rate;
        float voice_quality;
        
        ProsodicFeatures() : pitch_mean(0), pitch_variance(0), energy_mean(0),
                            energy_variance(0), speaking_rate(0), voice_quality(0) {}
    };
    
    EmotionRecognitionSpeech() {}
    
    // Extract prosodic features from audio
    ProsodicFeatures extract_features(const std::vector<float>& audio_signal, 
                                     float sample_rate = 16000.0f) {
        ProsodicFeatures features;
        
        // Pitch estimation (simplified autocorrelation)
        features.pitch_mean = estimate_pitch(audio_signal, sample_rate);
        features.pitch_variance = compute_pitch_variance(audio_signal, sample_rate);
        
        // Energy features
        features.energy_mean = compute_energy_mean(audio_signal);
        features.energy_variance = compute_energy_variance(audio_signal);
        
        // Speaking rate (zero-crossing rate proxy)
        features.speaking_rate = compute_zero_crossing_rate(audio_signal);
        
        // Voice quality (spectral centroid proxy)
        features.voice_quality = compute_spectral_centroid(audio_signal);
        
        return features;
    }
    
    // Recognize emotion from prosodic features
    Emotion recognize(const ProsodicFeatures& features) {
        // Rule-based classification using prosodic cues
        
        // Happy: high pitch, high energy, fast rate
        if (features.pitch_mean > 200 && features.energy_mean > 0.5f && 
            features.speaking_rate > 0.3f) {
            return Emotion::HAPPY;
        }
        
        // Sad: low pitch, low energy, slow rate
        if (features.pitch_mean < 150 && features.energy_mean < 0.3f && 
            features.speaking_rate < 0.2f) {
            return Emotion::SAD;
        }
        
        // Angry: high pitch variance, high energy, fast rate
        if (features.pitch_variance > 100 && features.energy_mean > 0.6f && 
            features.speaking_rate > 0.35f) {
            return Emotion::ANGRY;
        }
        
        // Fear: high pitch, high variance, moderate energy
        if (features.pitch_mean > 220 && features.pitch_variance > 80) {
            return Emotion::FEARFUL;
        }
        
        return Emotion::NEUTRAL;
    }
    
    std::string emotion_to_string(Emotion emotion) const {
        switch (emotion) {
            case Emotion::HAPPY: return "Happy";
            case Emotion::SAD: return "Sad";
            case Emotion::ANGRY: return "Angry";
            case Emotion::FEARFUL: return "Fearful";
            case Emotion::SURPRISED: return "Surprised";
            case Emotion::DISGUSTED: return "Disgusted";
            default: return "Neutral";
        }
    }
    
private:
    float estimate_pitch(const std::vector<float>& signal, float sample_rate) {
        // Simplified pitch estimation
        float sum = 0.0f;
        for (float s : signal) sum += std::abs(s);
        float avg = sum / signal.size();
        return 100.0f + avg * 200.0f;  // Map to pitch range
    }
    
    float compute_pitch_variance(const std::vector<float>& signal, float sample_rate) {
        float mean = estimate_pitch(signal, sample_rate);
        float variance = 0.0f;
        
        for (size_t i = 0; i < signal.size() - 100; i += 100) {
            std::vector<float> frame(signal.begin() + i, signal.begin() + i + 100);
            float pitch = estimate_pitch(frame, sample_rate);
            variance += (pitch - mean) * (pitch - mean);
        }
        
        return variance / (signal.size() / 100);
    }
    
    float compute_energy_mean(const std::vector<float>& signal) {
        float energy = 0.0f;
        for (float s : signal) {
            energy += s * s;
        }
        return energy / signal.size();
    }
    
    float compute_energy_variance(const std::vector<float>& signal) {
        float mean = compute_energy_mean(signal);
        float variance = 0.0f;
        
        for (float s : signal) {
            float e = s * s;
            variance += (e - mean) * (e - mean);
        }
        
        return variance / signal.size();
    }
    
    float compute_zero_crossing_rate(const std::vector<float>& signal) {
        int crossings = 0;
        for (size_t i = 1; i < signal.size(); i++) {
            if ((signal[i] >= 0 && signal[i-1] < 0) || 
                (signal[i] < 0 && signal[i-1] >= 0)) {
                crossings++;
            }
        }
        return static_cast<float>(crossings) / signal.size();
    }
    
    float compute_spectral_centroid(const std::vector<float>& signal) {
        // Simplified spectral centroid
        float weighted_sum = 0.0f;
        float sum = 0.0f;
        
        for (size_t i = 0; i < signal.size(); i++) {
            float magnitude = std::abs(signal[i]);
            weighted_sum += i * magnitude;
            sum += magnitude;
        }
        
        return sum > 0 ? weighted_sum / sum : 0.0f;
    }
};

} // namespace dnn::perception
