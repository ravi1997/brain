#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <unordered_map>

namespace dnn::perception {

// Environmental Sound Classification
class EnvironmentalSoundClassification {
public:
    struct AudioFeatures {
        float zero_crossing_rate;
        float spectral_centroid;
        float spectral_rolloff;
        std::vector<float> mfcc;  // Mel-frequency cepstral coefficients
        float energy;
        float tempo;
        
        AudioFeatures() : zero_crossing_rate(0), spectral_centroid(0),
                         spectral_rolloff(0), energy(0), tempo(0) {
            mfcc.resize(13, 0.0f);
        }
    };
    
    EnvironmentalSoundClassification(float sample_rate = 44100.0f)
        : sample_rate_(sample_rate) {
        initialize_classes();
    }
    
    // Classify environmental sound
    std::string classify(const std::vector<float>& audio) {
        auto features = extract_features(audio);
        return classify_from_features(features);
    }
    
    // Extract audio features
    AudioFeatures extract_features(const std::vector<float>& audio) {
        AudioFeatures features;
        
        features.zero_crossing_rate = compute_zcr(audio);
        features.spectral_centroid = compute_spectral_centroid(audio);
        features.spectral_rolloff = compute_spectral_rolloff(audio);
        features.mfcc = compute_mfcc(audio);
        features.energy = compute_energy(audio);
        features.tempo = estimate_tempo(audio);
        
        return features;
    }
    
    // Get confidence scores for all classes
    std::unordered_map<std::string, float> classify_with_confidence(
        const std::vector<float>& audio) {
        auto features = extract_features(audio);
        return compute_class_scores(features);
    }
    
private:
    float sample_rate_;
    std::vector<std::string> sound_classes_;
    
    void initialize_classes() {
        sound_classes_ = {
            "speech",
            "music",
            "traffic",
            "siren",
            "dog_bark",
            "footsteps",
            "door",
            "water",
            "wind",
            "birds",
            "appliance",
            "silence"
        };
    }
    
    std::string classify_from_features(const AudioFeatures& features) {
        auto scores = compute_class_scores(features);
        
        std::string best_class;
        float best_score = 0.0f;
        
        for (const auto& [cls, score] : scores) {
            if (score > best_score) {
                best_score = score;
                best_class = cls;
            }
        }
        
        return best_class;
    }
    
    std::unordered_map<std::string, float> compute_class_scores(
        const AudioFeatures& features) {
        std::unordered_map<std::string, float> scores;
        
        // Rule-based classification (simplified)
        
        // Silence
        scores["silence"] = (features.energy < 0.01f) ? 0.9f : 0.1f;
        
        // Speech - moderate ZCR, mid-range spectrum
        if (features.zero_crossing_rate > 0.1f && features.zero_crossing_rate < 0.3f &&
            features.spectral_centroid > 1500 && features.spectral_centroid < 4000) {
            scores["speech"] = 0.7f;
        } else {
            scores["speech"] = 0.2f;
        }
        
        // Music - rhythmic, rich spectrum
        if (features.tempo > 60 && features.tempo < 180 &&
            features.spectral_rolloff > 5000) {
            scores["music"] = 0.8f;
        } else {
            scores["music"] = 0.2f;
        }
        
        // Traffic - low-frequency rumble
        if (features.spectral_centroid < 800 && features.energy > 0.1f) {
            scores["traffic"] = 0.7f;
        } else {
            scores["traffic"] = 0.1f;
        }
        
        // Siren - high-frequency, varying
        if (features.spectral_centroid > 2000 && 
            features.zero_crossing_rate > 0.4f) {
            scores["siren"] = 0.8f;
        } else {
            scores["siren"] = 0.1f;
        }
        
        // Dog bark - impulsive, mid-high frequency
        if (features.energy > 0.3f && features.spectral_centroid > 1000 &&
            features.spectral_centroid < 3000) {
            scores["dog_bark"] = 0.6f;
        } else {
            scores["dog_bark"] = 0.15f;
        }
        
        // Footsteps - rhythmic, low energy bursts
        if (features.tempo > 0 && features.tempo < 200 &&
            features.energy < 0.3f && features.spectral_centroid < 1500) {
            scores["footsteps"] = 0.6f;
        } else {
            scores["footsteps"] = 0.1f;
        }
        
        // Door - transient, broadband
        if (features.energy > 0.2f && features.spectral_rolloff > 6000) {
            scores["door"] = 0.5f;
        } else {
            scores["door"] = 0.1f;
        }
        
        // Water - continuous, noise-like, mid-high frequency
        if (features.zero_crossing_rate > 0.3f &&
            features.spectral_centroid > 2000 && features.spectral_centroid < 6000) {
            scores["water"] = 0.6f;
        } else {
            scores["water"] = 0.1f;
        }
        
        // Wind - low-frequency noise
        if (features.spectral_centroid < 1000 && features.zero_crossing_rate > 0.2f) {
            scores["wind"] = 0.6f;
        } else {
            scores["wind"] = 0.1f;
        }
        
        // Birds - high-pitched, varying
        if (features.spectral_centroid > 3000 && features.zero_crossing_rate > 0.3f) {
            scores["birds"] = 0.7f;
        } else {
            scores["birds"] = 0.1f;
        }
        
        // Appliance - steady, humming
        if (features.spectral_centroid > 500 && features.spectral_centroid < 2000 &&
            features.energy > 0.05f && features.zero_crossing_rate < 0.25f) {
            scores["appliance"] = 0.6f;
        } else {
            scores["appliance"] = 0.1f;
        }
        
        // Normalize scores
        float total = 0.0f;
        for (const auto& [_, score] : scores) {
            total += score;
        }
        
        if (total > 0) {
            for (auto& [_, score] : scores) {
                score /= total;
            }
        }
        
        return scores;
    }
    
    float compute_zcr(const std::vector<float>& audio) {
        int zero_crossings = 0;
        
        for (size_t i = 1; i < audio.size(); i++) {
            if ((audio[i] >= 0 && audio[i-1] < 0) ||
                (audio[i] < 0 && audio[i-1] >= 0)) {
                zero_crossings++;
            }
        }
        
        return audio.empty() ? 0.0f : static_cast<float>(zero_crossings) / audio.size();
    }
    
    float compute_spectral_centroid(const std::vector<float>& audio) {
        // Simplified: weighted frequency average
        int fft_size = std::min(2048, static_cast<int>(audio.size()));
        
        float weighted_sum = 0.0f;
        float magnitude_sum = 0.0f;
        
        for (int k = 0; k < fft_size / 2; k++) {
            float freq = k * sample_rate_ / fft_size;
            float magnitude = (k < static_cast<int>(audio.size())) ? 
                             std::abs(audio[k]) : 0.0f;
            
            weighted_sum += freq * magnitude;
            magnitude_sum += magnitude;
        }
        
        return magnitude_sum > 0 ? weighted_sum / magnitude_sum : 0.0f;
    }
    
    float compute_spectral_rolloff(const std::vector<float>& audio) {
        // Frequency below which 85% of energy is concentrated
        int fft_size = std::min(2048, static_cast<int>(audio.size()));
        
        std::vector<float> magnitudes;
        float total_energy = 0.0f;
        
        for (int k = 0; k < fft_size / 2; k++) {
            float mag = (k < static_cast<int>(audio.size())) ? 
                       std::abs(audio[k]) : 0.0f;
            magnitudes.push_back(mag);
            total_energy += mag;
        }
        
        float cumulative = 0.0f;
        float threshold = 0.85f * total_energy;
        
        for (size_t k = 0; k < magnitudes.size(); k++) {
            cumulative += magnitudes[k];
            if (cumulative >= threshold) {
                return k * sample_rate_ / fft_size;
            }
        }
        
        return sample_rate_ / 2;
    }
    
    std::vector<float> compute_mfcc(const std::vector<float>& audio) {
        // Simplified MFCC computation
        std::vector<float> mfcc(13, 0.0f);
        
        // Would normally: FFT -> Mel filterbank -> Log -> DCT
        // Here: simplified energy distribution
        int num_bands = 13;
        int band_size = audio.size() / num_bands;
        
        for (int i = 0; i < num_bands && i * band_size < static_cast<int>(audio.size()); i++) {
            float energy = 0.0f;
            for (int j = 0; j < band_size && i * band_size + j < static_cast<int>(audio.size()); j++) {
                energy += audio[i * band_size + j] * audio[i * band_size + j];
            }
            mfcc[i] = std::log(1.0f + energy);
        }
        
        return mfcc;
    }
    
    float compute_energy(const std::vector<float>& audio) {
        float energy = 0.0f;
        for (float sample : audio) {
            energy += sample * sample;
        }
        return std::sqrt(energy / audio.size());
    }
    
    float estimate_tempo(const std::vector<float>& audio) {
        // Autocorrelation-based tempo
        int max_lag = static_cast<int>(sample_rate_ * 0.5f);  // 120 BPM min
        
        float max_correlation = 0.0f;
        int best_lag = 0;
        
        for (int lag = static_cast<int>(sample_rate_ * 0.25f); 
             lag < max_lag && lag < static_cast<int>(audio.size()); lag++) {
            float correlation = 0.0f;
            
            for (size_t i = 0; i + lag < audio.size(); i++) {
                correlation += audio[i] * audio[i + lag];
            }
            
            if (correlation > max_correlation) {
                max_correlation = correlation;
                best_lag = lag;
            }
        }
        
        if (best_lag == 0) return 0.0f;
        
        float period = best_lag / sample_rate_;
        return 60.0f / period;  // BPM
    }
};

} // namespace dnn::perception
