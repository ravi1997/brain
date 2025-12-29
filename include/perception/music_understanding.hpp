#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <unordered_map>

namespace dnn::perception {

// Music Understanding - analyze musical structure and content
class MusicUnderstanding {
public:
    struct MusicFeatures {
        float tempo;  // BPM
        std::string key;  // C, D, E, etc.
        std::string mode;  // major, minor
        std::vector<float> chroma;  // 12-dimensional chroma features
        float energy;
        float valence;  // happiness/sadness
        std::vector<int> beat_positions;
        
        MusicFeatures() : tempo(120), mode("major"), energy(0.5f), valence(0.5f) {
            chroma.resize(12, 0.0f);
        }
    };
    
    struct Chord {
        std::string root;  // C, D, E, F, G, A, B
        std::string quality;  // major, minor, dim, aug
        float timestamp;
        
        Chord() : timestamp(0) {}
        Chord(const std::string& r, const std::string& q, float t)
            : root(r), quality(q), timestamp(t) {}
    };
    
    MusicUnderstanding(float sample_rate = 44100.0f) 
        : sample_rate_(sample_rate) {
        initialize_notes();
    }
    
    // Extract musical features from audio
    MusicFeatures analyze(const std::vector<float>& audio) {
        MusicFeatures features;
        
        // Tempo estimation
        features.tempo = estimate_tempo(audio);
        
        // Beat detection
        features.beat_positions = detect_beats(audio, features.tempo);
        
        // Chroma features
        features.chroma = compute_chroma(audio);
        
        // Key detection
        features.key = detect_key(features.chroma);
        features.mode = detect_mode(features.chroma);
        
        // Energy
        features.energy = compute_energy(audio);
        
        // Valence (happiness) - simplified
        features.valence = compute_valence(features.chroma, features.mode);
        
        return features;
    }
    
    // Chord recognition
    std::vector<Chord> recognize_chords(const std::vector<float>& audio, 
                                       float hop_seconds = 0.5f) {
        std::vector<Chord> chords;
        
        int hop_samples = static_cast<int>(hop_seconds * sample_rate_);
        
        for (size_t pos = 0; pos + hop_samples < audio.size(); pos += hop_samples) {
            // Extract segment
            std::vector<float> segment(audio.begin() + pos,
                                      audio.begin() + pos + hop_samples);
            
            // Compute chroma
            auto chroma = compute_chroma(segment);
            
            // Recognize chord
            auto chord = chroma_to_chord(chroma);
            chord.timestamp = pos / sample_rate_;
            
            chord.push_back(chord);
        }
        
        return chords;
    }
    
    // Genre classification
    std::string classify_genre(const MusicFeatures& features) {
        // Rule-based classification (simplified)
        
        if (features.tempo > 140 && features.energy > 0.7f) {
            return "Electronic/Dance";
        }
        
        if (features.tempo < 90 && features.valence < 0.4f) {
            return "Blues/Ballad";
        }
        
        if (features.mode == "minor" && features.energy > 0.6f) {
            return "Rock/Metal";
        }
        
        if (features.mode == "major" && features.valence > 0.6f) {
            return "Pop";
        }
        
        if (features.tempo > 100 && features.tempo < 130) {
            return "Jazz/Funk";
        }
        
        return "Classical/Other";
    }
    
    // Similarity between two pieces
    float compute_similarity(const MusicFeatures& f1, const MusicFeatures& f2) {
        float similarity = 0.0f;
        
        // Tempo similarity
        float tempo_diff = std::abs(f1.tempo - f2.tempo) / 200.0f;
        similarity += (1.0f - tempo_diff) * 0.2f;
        
        // Chroma similarity (cosine)
        float chroma_sim = cosine_similarity(f1.chroma, f2.chroma);
        similarity += chroma_sim * 0.4f;
        
        // Mode similarity
        if (f1.mode == f2.mode) {
            similarity += 0.2f;
        }
        
        // Energy similarity
        float energy_diff = std::abs(f1.energy - f2.energy);
        similarity += (1.0f - energy_diff) * 0.2f;
        
        return similarity;
    }
    
private:
    float sample_rate_;
    std::unordered_map<std::string, int> note_to_index_;
    std::vector<std::string> index_to_note_;
    
    void initialize_notes() {
        std::vector<std::string> notes = {"C", "C#", "D", "D#", "E", "F", 
                                         "F#", "G", "G#", "A", "A#", "B"};
        index_to_note_ = notes;
        
        for (size_t i = 0; i < notes.size(); i++) {
            note_to_index_[notes[i]] = i;
        }
    }
    
    float estimate_tempo(const std::vector<float>& audio) {
        // Autocorrelation-based tempo estimation
        int min_lag = static_cast<int>(sample_rate_ * 0.3f);  // 200 BPM max
        int max_lag = static_cast<int>(sample_rate_ * 1.2f);  // 50 BPM min
        
        float max_correlation = 0.0f;
        int best_lag = min_lag;
        
        // Compute energy envelope
        std::vector<float> envelope;
        int window = 1024;
        
        for (size_t i = 0; i + window < audio.size(); i += window / 4) {
            float energy = 0.0f;
            for (int j = 0; j < window; j++) {
                energy += audio[i + j] * audio[i + j];
            }
            envelope.push_back(std::sqrt(energy));
        }
        
        // Find periodicity in envelope
        for (int lag = min_lag / (window / 4); lag <= max_lag / (window / 4) && 
             lag < static_cast<int>(envelope.size()); lag++) {
            float correlation = 0.0f;
            
            for (size_t i = 0; i + lag < envelope.size(); i++) {
                correlation += envelope[i] * envelope[i + lag];
            }
            
            if (correlation > max_correlation) {
                max_correlation = correlation;
                best_lag = lag;
            }
        }
        
        float period_seconds = (best_lag * window / 4.0f) / sample_rate_;
        float tempo = 60.0f / period_seconds;
        
        return std::max(50.0f, std::min(200.0f, tempo));
    }
    
    std::vector<int> detect_beats(const std::vector<float>& audio, float tempo) {
        std::vector<int> beats;
        
        float beat_interval = 60.0f / tempo;  // seconds
        int beat_samples = static_cast<int>(beat_interval * sample_rate_);
        
        for (int pos = 0; pos < static_cast<int>(audio.size()); pos += beat_samples) {
            beats.push_back(pos);
        }
        
        return beats;
    }
    
    std::vector<float> compute_chroma(const std::vector<float>& audio) {
        std::vector<float> chroma(12, 0.0f);
        
        // Simplified: bin energies into 12 pitch classes
        int fft_size = 4096;
        
        for (size_t i = 0; i + fft_size < audio.size(); i += fft_size / 2) {
            // Compute magnitude spectrum (simplified)
            for (int k = 1; k < fft_size / 2; k++) {
                float freq = k * sample_rate_ / fft_size;
                
                // Map frequency to pitch class
                if (freq > 80 && freq < 2000) {  // Musical range
                    int pitch_class = freq_to_pitch_class(freq);
                    float magnitude = std::abs(audio[i + k]);
                    chroma[pitch_class] += magnitude;
                }
            }
        }
        
        // Normalize
        float max_val = *std::max_element(chroma.begin(), chroma.end());
        if (max_val > 0) {
            for (auto& c : chroma) {
                c /= max_val;
            }
        }
        
        return chroma;
    }
    
    std::string detect_key(const std::vector<float>& chroma) {
        // Find pitch class with highest energy
        int max_idx = std::max_element(chroma.begin(), chroma.end()) - chroma.begin();
        return index_to_note_[max_idx];
    }
    
    std::string detect_mode(const std::vector<float>& chroma) {
        // Major has strong 3rd (4 semitones up), minor has flat 3rd (3 semitones)
        int root = std::max_element(chroma.begin(), chroma.end()) - chroma.begin();
        
        int major_third = (root + 4) % 12;
        int minor_third = (root + 3) % 12;
        
        return chroma[major_third] > chroma[minor_third] ? "major" : "minor";
    }
    
    float compute_energy(const std::vector<float>& audio) {
        float energy = 0.0f;
        for (float sample : audio) {
            energy += sample * sample;
        }
        return std::sqrt(energy / audio.size());
    }
    
    float compute_valence(const std::vector<float>& chroma, const std::string& mode) {
        // Major tends to be happier
        float base_valence = (mode == "major") ? 0.6f : 0.4f;
        
        // Brighter chroma = happier
        float brightness = 0.0f;
        for (size_t i = 0; i < chroma.size(); i++) {
            brightness += chroma[i] * (i / 12.0f);
        }
        
        return std::max(0.0f, std::min(1.0f, base_valence + brightness * 0.2f));
    }
    
    Chord chroma_to_chord(const std::vector<float>& chroma) {
        Chord chord;
        
        // Find root (highest chroma)
        int root_idx = std::max_element(chroma.begin(), chroma.end()) - chroma.begin();
        chord.root = index_to_note_[root_idx];
        
        // Determine quality
        int third_major = (root_idx + 4) % 12;
        int third_minor = (root_idx + 3) % 12;
        
        if (chroma[third_major] > chroma[third_minor]) {
            chord.quality = "major";
        } else {
            chord.quality = "minor";
        }
        
        return chord;
    }
    
    int freq_to_pitch_class(float freq) {
        // Convert frequency to MIDI note, then to pitch class
        float midi = 69 + 12 * std::log2(freq / 440.0f);
        int pitch_class = static_cast<int>(std::round(midi)) % 12;
        return pitch_class;
    }
    
    float cosine_similarity(const std::vector<float>& v1, const std::vector<float>& v2) {
        float dot = 0.0f, norm1 = 0.0f, norm2 = 0.0f;
        
        for (size_t i = 0; i < std::min(v1.size(), v2.size()); i++) {
            dot += v1[i] * v2[i];
            norm1 += v1[i] * v1[i];
            norm2 += v2[i] * v2[i];
        }
        
        if (norm1 == 0 || norm2 == 0) return 0.0f;
        
        return dot / (std::sqrt(norm1) * std::sqrt(norm2));
    }
};

} // namespace dnn::perception
