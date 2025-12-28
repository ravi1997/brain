#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>

namespace dnn::perception {

// Speaker diarization - "who spoke when"
class SpeakerDiarization {
public:
    struct Segment {
        float start_time;
        float end_time;
        int speaker_id;
        
        Segment() : start_time(0), end_time(0), speaker_id(0) {}
        Segment(float s, float e, int id) : start_time(s), end_time(e), speaker_id(id) {}
    };
    
    SpeakerDiarization(int num_speakers = 2) : num_speakers_(num_speakers) {}
    
    // Diarize audio into speaker segments
    std::vector<Segment> diarize(const std::vector<float>& audio, float sample_rate = 16000.0f) {
        std::vector<Segment> segments;
        
        // Simple energy-based segmentation
        float segment_duration = 1.0f;  // 1 second segments
        int segment_samples = static_cast<int>(segment_duration * sample_rate);
        
        int current_speaker = 0;
        float segment_start = 0.0f;
        
        for (size_t i = 0; i < audio.size(); i += segment_samples) {
            // Compute energy for this segment
            float energy = 0.0f;
            for (size_t j = i; j < i + segment_samples && j < audio.size(); j++) {
                energy += audio[j] * audio[j];
            }
            energy /= segment_samples;
            
            // Assign speaker based on energy level (simplified)
            int speaker = static_cast<int>(energy * num_speakers_) % num_speakers_;
            
            if (speaker != current_speaker) {
                // New speaker segment
                float segment_end = static_cast<float>(i) / sample_rate;
                if (segment_end > segment_start) {
                    segments.emplace_back(segment_start, segment_end, current_speaker);
                }
                segment_start = segment_end;
                current_speaker = speaker;
            }
        }
        
        // Add final segment
        float final_time = static_cast<float>(audio.size()) / sample_rate;
        if (final_time > segment_start) {
            segments.emplace_back(segment_start, final_time, current_speaker);
        }
        
        return segments;
    }
    
    // Get total speaking time per speaker
    std::vector<float> get_speaker_durations(const std::vector<Segment>& segments) {
        std::vector<float> durations(num_speakers_, 0.0f);
        
        for (const auto& seg : segments) {
            if (seg.speaker_id >= 0 && seg.speaker_id < num_speakers_) {
                durations[seg.speaker_id] += (seg.end_time - seg.start_time);
            }
        }
        
        return durations;
    }
    
private:
    int num_speakers_;
};

} // namespace dnn::perception
