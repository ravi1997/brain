import { BrainClient } from '../core/client.js';

export const VoiceFeature = {
    recognition: null,
    synth: window.speechSynthesis,
    isListening: false,

    init() {
        // UI Setup
        this.createMicButton();

        // STT Setup
        if ('webkitSpeechRecognition' in window) {
            this.recognition = new webkitSpeechRecognition();
            this.recognition.continuous = false;
            this.recognition.interimResults = false;
            this.recognition.lang = 'en-US';

            this.recognition.onresult = (e) => {
                const text = e.results[0][0].transcript;
                console.log(`[Voice] Heard: ${text}`);
                this.handleVoiceInput(text);
            };

            this.recognition.onerror = (e) => {
                console.error('[Voice] Error:', e.error);
                this.setListening(false);
            };

            this.recognition.onend = () => {
                this.setListening(false);
            };
        } else {
            console.warn("[Voice] Web Speech API not supported.");
        }

        // TTS Integration: Listen to 'chat' from Brain
        BrainClient.subscribe('chat', (data) => {
            if (data.startsWith('Brain: ')) {
                const text = data.replace('Brain: ', '');
                // Only speak if we interacted recently or if "Always Speak" mode is on (simplified: speak all)
                // For now, let's just speak connections initiated by voice.
                // But since we can't easily track that state across modules perfectly without a store, 
                // we'll speak everything if the Mic was recently active or if we add a toggle.
                // Let's just speak it if it's short, or always?
                // Better UX: Only speak if the last input was voice? 
                // Let's stick to a simple strategy: If dashboard is open, speak it.
                this.speak(text);
            }
        });
    },

    createMicButton() {
        // Find header to inject
        const header = document.querySelector('.top-bar');
        if (!header) return;

        const btn = document.createElement('button');
        btn.id = 'mic-btn';
        btn.innerHTML = '<i class="fa-solid fa-microphone"></i>';
        btn.className = 'mic-btn';
        btn.style.marginRight = '20px';
        btn.onclick = () => this.toggleListen();

        // Inject before the ticker
        header.insertBefore(btn, header.lastElementChild);
    },

    toggleListen() {
        if (!this.recognition) return alert("Browser does not support Voice.");

        if (this.isListening) {
            this.recognition.stop();
            this.setListening(false);
        } else {
            try {
                this.recognition.start();
                this.setListening(true);
            } catch (e) { console.error(e); }
        }
    },

    setListening(active) {
        this.isListening = active;
        const btn = document.getElementById('mic-btn');
        if (btn) {
            if (active) {
                btn.classList.add('recording');
                btn.style.color = '#ff5555';
                btn.style.textShadow = '0 0 10px #ff5555';
            } else {
                btn.classList.remove('recording');
                btn.style.color = '';
                btn.style.textShadow = '';
            }
        }
    },

    handleVoiceInput(text) {
        // Feedback in Terminal (if open) or just Send
        BrainClient.send('chat', text);

        // Visual feedback
        const ticker = document.getElementById('ticker-text');
        if (ticker) ticker.textContent = `[VOICE] ${text}`;
    },

    speak(text) {
        if (this.synth.speaking) this.synth.cancel();

        const utter = new SpeechSynthesisUtterance(text);
        utter.pitch = 1.0;
        utter.rate = 1.1;

        // Try to find a cool voice
        const voices = this.synth.getVoices();
        const robotVoice = voices.find(v => v.name.includes('Google US English') || v.name.includes('Samantha'));
        if (robotVoice) utter.voice = robotVoice;

        this.synth.speak(utter);
    }
};
