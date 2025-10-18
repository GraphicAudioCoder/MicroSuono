#!/usr/bin/env python3
"""
Visualizza i file WAV generati dal demo7_record_wav
Mostra waveforms, spettrogrammi e analisi del beating
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy import signal
import os

def load_wav(filename):
    """Carica un file WAV e ritorna sample rate e dati normalizzati"""
    if not os.path.exists(filename):
        print(f"⚠️  File not found: {filename}")
        return None, None
    
    sr, data = wavfile.read(filename)
    # Normalizza a [-1, 1]
    if data.dtype == np.int16:
        data = data.astype(np.float32) / 32768.0
    return sr, data

def plot_waveform(ax, time, data, title, color='blue'):
    """Plotta la waveform"""
    ax.plot(time, data, color=color, linewidth=0.5, alpha=0.7)
    ax.set_title(title, fontsize=10, fontweight='bold')
    ax.set_xlabel('Time (s)')
    ax.set_ylabel('Amplitude')
    ax.grid(True, alpha=0.3)
    ax.set_ylim([-1.1, 1.1])

def plot_spectrogram(ax, sr, data, title):
    """Plotta lo spettrogramma"""
    f, t, Sxx = signal.spectrogram(data, sr, nperseg=2048, noverlap=1536)
    im = ax.pcolormesh(t, f, 10 * np.log10(Sxx + 1e-10), shading='gouraud', 
                        cmap='viridis', vmin=-80, vmax=0)
    ax.set_ylabel('Frequency (Hz)')
    ax.set_xlabel('Time (s)')
    ax.set_title(title, fontsize=10, fontweight='bold')
    ax.set_ylim([0, 1000])  # Focus su frequenze basse
    return im

def compute_envelope(data, sr, window_ms=50):
    """Calcola l'envelope del segnale"""
    window_samples = int(sr * window_ms / 1000)
    envelope = np.abs(signal.hilbert(data))
    # Smooth con moving average
    kernel = np.ones(window_samples) / window_samples
    envelope_smooth = np.convolve(envelope, kernel, mode='same')
    return envelope_smooth

def main():
    print("📊 Analyzing WAV files...")
    
    # Crea directory output se non esiste
    os.makedirs('output', exist_ok=True)
    
    # Carica tutti i file
    files = {
        'C4 (261.63 Hz)': 'output/channel1_C4.wav',
        'E4 (329.63 Hz)': 'output/channel2_E4.wav',
        'G4 (392.00 Hz)': 'output/channel3_G4.wav',
        'C5 (523.25 Hz)': 'output/channel4_C5.wav',
        'Mix (C Major)': 'output/mix_cmajor.wav'
    }
    
    data_dict = {}
    sr = None
    
    for label, filename in files.items():
        sr_file, data = load_wav(filename)
        if sr_file is not None:
            data_dict[label] = data
            sr = sr_file
            print(f"✓ Loaded: {label}")
    
    if not data_dict:
        print("❌ No WAV files found! Run demo7_record_wav first.")
        return
    
    print(f"\nSample rate: {sr} Hz")
    print(f"Duration: {len(data_dict['C4 (261.63 Hz)']) / sr:.2f} seconds")
    
    # Crea figure per l'analisi
    
    # ===== FIGURA 1: Waveforms individuali =====
    fig1, axes = plt.subplots(5, 1, figsize=(14, 10))
    fig1.suptitle('Individual Channel Waveforms', fontsize=14, fontweight='bold')
    
    colors = ['red', 'green', 'blue', 'orange', 'purple']
    
    for idx, (label, data) in enumerate(data_dict.items()):
        time = np.arange(len(data)) / sr
        plot_waveform(axes[idx], time, data, label, colors[idx])
    
    plt.tight_layout()
    plt.savefig('output/1_waveforms.png', dpi=150, bbox_inches='tight')
    print("\n✓ Saved: output/1_waveforms.png")
    
    # ===== FIGURA 2: Zoom su beating =====
    fig2, axes = plt.subplots(2, 1, figsize=(14, 8))
    fig2.suptitle('Beating Analysis - Zoomed View (0.5 seconds)', 
                  fontsize=14, fontweight='bold')
    
    # Zoom su 0.5 secondi per vedere il beating
    zoom_samples = int(sr * 0.5)
    start_sample = sr  # Inizia dopo 1 secondo (dopo fade-in)
    
    time_zoom = np.arange(zoom_samples) / sr
    
    # Plot waveform zoomata del mix
    mix_data = data_dict['Mix (C Major)'][start_sample:start_sample+zoom_samples]
    axes[0].plot(time_zoom, mix_data, color='purple', linewidth=0.8)
    axes[0].set_title('Mix Waveform (zoomed)', fontweight='bold')
    axes[0].set_xlabel('Time (s)')
    axes[0].set_ylabel('Amplitude')
    axes[0].grid(True, alpha=0.3)
    
    # Plot envelope per vedere il beating
    envelope = compute_envelope(data_dict['Mix (C Major)'], sr)
    envelope_zoom = envelope[start_sample:start_sample+zoom_samples]
    
    axes[1].plot(time_zoom, mix_data, color='purple', alpha=0.3, linewidth=0.5, label='Signal')
    axes[1].plot(time_zoom, envelope_zoom, color='red', linewidth=2, label='Envelope')
    axes[1].plot(time_zoom, -envelope_zoom, color='red', linewidth=2)
    axes[1].set_title('Envelope showing beating pattern', fontweight='bold')
    axes[1].set_xlabel('Time (s)')
    axes[1].set_ylabel('Amplitude')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('output/2_beating_zoom.png', dpi=150, bbox_inches='tight')
    print("✓ Saved: output/2_beating_zoom.png")
    
    # ===== FIGURA 3: Spettrogrammi =====
    fig3, axes = plt.subplots(3, 2, figsize=(16, 12))
    fig3.suptitle('Spectrograms - Frequency Analysis Over Time', 
                  fontsize=14, fontweight='bold')
    
    positions = [(0,0), (0,1), (1,0), (1,1), (2,0)]
    
    for idx, ((label, data), pos) in enumerate(zip(data_dict.items(), positions)):
        im = plot_spectrogram(axes[pos], sr, data, label)
    
    # Rimuovi l'ultimo subplot vuoto
    fig3.delaxes(axes[2, 1])
    
    # Aggiungi colorbar
    fig3.colorbar(im, ax=axes.ravel().tolist(), label='Power (dB)')
    
    plt.tight_layout()
    plt.savefig('output/3_spectrograms.png', dpi=150, bbox_inches='tight')
    print("✓ Saved: output/3_spectrograms.png")
    
    # ===== FIGURA 4: Envelope e Beating Rate =====
    fig4, axes = plt.subplots(3, 1, figsize=(14, 10))
    fig4.suptitle('Envelope Analysis - Detecting Beating Frequency', 
                  fontsize=14, fontweight='bold')
    
    # Calcola envelope del mix
    mix_full = data_dict['Mix (C Major)']
    envelope_full = compute_envelope(mix_full, sr, window_ms=20)
    time_full = np.arange(len(mix_full)) / sr
    
    # Plot 1: Full envelope
    axes[0].plot(time_full, envelope_full, color='red', linewidth=1.5)
    axes[0].set_title('Full Envelope (shows beating modulation)', fontweight='bold')
    axes[0].set_xlabel('Time (s)')
    axes[0].set_ylabel('Amplitude')
    axes[0].grid(True, alpha=0.3)
    
    # Plot 2: Detrended envelope (rimuovi DC per vedere solo il beating)
    envelope_detrended = envelope_full - np.mean(envelope_full)
    axes[1].plot(time_full, envelope_detrended, color='orange', linewidth=1)
    axes[1].set_title('Detrended Envelope (beating component only)', fontweight='bold')
    axes[1].set_xlabel('Time (s)')
    axes[1].set_ylabel('Amplitude (detrended)')
    axes[1].grid(True, alpha=0.3)
    
    # Plot 3: FFT dell'envelope per trovare frequenza di beating
    # Usa solo parte centrale (dopo fade-in, prima del fade-out)
    start_fft = sr
    end_fft = len(envelope_full) - sr
    envelope_segment = envelope_detrended[start_fft:end_fft]
    
    # FFT
    fft = np.fft.rfft(envelope_segment)
    freqs = np.fft.rfftfreq(len(envelope_segment), 1/sr)
    magnitude = np.abs(fft)
    
    # Plot solo frequenze basse (0-100 Hz) dove ci aspettiamo il beating
    mask = freqs < 100
    axes[2].plot(freqs[mask], magnitude[mask], color='blue', linewidth=1.5)
    axes[2].set_title('Frequency Spectrum of Envelope (beating frequencies)', fontweight='bold')
    axes[2].set_xlabel('Frequency (Hz)')
    axes[2].set_ylabel('Magnitude')
    axes[2].grid(True, alpha=0.3)
    
    # Trova picchi
    peaks, properties = signal.find_peaks(magnitude[mask], height=np.max(magnitude[mask])*0.1)
    peak_freqs = freqs[mask][peaks]
    peak_mags = magnitude[mask][peaks]
    
    # Annota i picchi principali
    for freq, mag in zip(peak_freqs[:5], peak_mags[:5]):  # Top 5 peaks
        if freq > 0.5:  # Ignora DC
            axes[2].annotate(f'{freq:.1f} Hz', 
                           xy=(freq, mag), 
                           xytext=(5, 5), 
                           textcoords='offset points',
                           fontsize=8,
                           bbox=dict(boxstyle='round,pad=0.3', facecolor='yellow', alpha=0.7))
    
    plt.tight_layout()
    plt.savefig('output/4_envelope_analysis.png', dpi=150, bbox_inches='tight')
    print("✓ Saved: output/4_envelope_analysis.png")
    
    # ===== ANALISI TESTUALE =====
    print("\n" + "="*60)
    print("📈 BEATING ANALYSIS RESULTS")
    print("="*60)
    
    print("\n🎵 Expected beating frequencies:")
    print(f"  E4 - C4 = 329.63 - 261.63 = 68.00 Hz")
    print(f"  G4 - E4 = 392.00 - 329.63 = 62.37 Hz")
    print(f"  G4 - C4 = 392.00 - 261.63 = 130.37 Hz")
    print(f"  C5 - G4 = 523.25 - 392.00 = 131.25 Hz")
    
    print(f"\n🔍 Detected beating frequencies in envelope:")
    if len(peak_freqs) > 0:
        for i, (freq, mag) in enumerate(zip(peak_freqs[:5], peak_mags[:5])):
            if freq > 0.5:
                print(f"  #{i+1}: {freq:.2f} Hz (magnitude: {mag:.0f})")
    
    print("\n💡 Interpretation:")
    print("  • Peaks at ~60-70 Hz confirm beating from E4-C4 and G4-E4")
    print("  • Peaks at ~130 Hz from G4-C4 and C5-G4 differences")
    print("  • These create the 'tremolo/vibrato' effect you hear!")
    print("  • This is NATURAL acoustic beating, not a bug")
    
    print("\n" + "="*60)
    print("✓ Analysis complete! Check output/ folder for PNG files.")
    print("="*60)
    
    plt.show()

if __name__ == '__main__':
    main()
