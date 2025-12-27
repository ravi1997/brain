import React, { useRef, useEffect, useState } from 'react';
import * as THREE from 'three';

/**
 * NeuronVisualizer - WebGL-based 3D visualization of neural network activity
 * Displays neurons as spheres, color-coded by activation level
 */
export default function NeuronVisualizer({ brainState }) {
  const canvasRef = useRef(null);
  const sceneRef = useRef(null);
  const neuronsRef = useRef([]);
  const rendererRef = useRef(null);
  const cameraRef = useRef(null);
  const [isLoaded, setIsLoaded] = useState(false);

  // Initialize Three.js scene
  useEffect(() => {
    if (!canvasRef.current) return;

    // Scene setup
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0a0a);
    sceneRef.current = scene;

    // Camera
    const camera = new THREE.PerspectiveCamera(
      75,
      canvasRef.current.clientWidth / canvasRef.current.clientHeight,
      0.1,
      1000
    );
    camera.position.z = 50;
    cameraRef.current = camera;

    // Renderer
    const renderer = new THREE.WebGLRenderer({ 
      canvas: canvasRef.current,
      antialias: true 
    });
    renderer.setSize(canvasRef.current.clientWidth, canvasRef.current.clientHeight);
    rendererRef.current = renderer;

    // Lighting
    const ambientLight = new THREE.AmbientLight(0x404040, 2);
    scene.add(ambientLight);
    const pointLight = new THREE.PointLight(0xffffff, 1, 100);
    pointLight.position.set(10, 10, 10);
    scene.add(pointLight);

    // Create neurons for 4 brain regions
    const regions = [
      { name: 'Encoder', color: 0x3b82f6, count: 256, position: [-20, 10, 0] },
      { name: 'Decoder', color: 0x8b5cf6, count: 256, position: [20, 10, 0] },
      { name: 'Memory', color: 0x10b981, count: 256, position: [-20, -10, 0] },
      { name: 'Cognitive', color: 0xf59e0b, count: 256, position: [20, -10, 0] }
    ];

    const neurons = [];
    const dummy = new THREE.Object3D();
    
    regions.forEach((region, regionIdx) => {
      const geometry = new THREE.SphereGeometry(0.3, 8, 8);
      const material = new THREE.MeshPhongMaterial({ 
          color: region.color,
          emissive: region.color,
          emissiveIntensity: 0.2
      });
      
      const instancedMesh = new THREE.InstancedMesh(geometry, material, region.count);
      instancedMesh.instanceMatrix.setUsage(THREE.DynamicDrawUsage);
      
      const gridSize = Math.ceil(Math.sqrt(region.count));
      const spacing = 1.5;
      
      for (let i = 0; i < region.count; i++) {
        const x = (i % gridSize) * spacing - (gridSize * spacing) / 2;
        const y = Math.floor(i / gridSize) * spacing - (gridSize * spacing) / 2;
        
        dummy.position.set(
          region.position[0] + x / 2,
          region.position[1] + y / 2,
          region.position[2]
        );
        dummy.updateMatrix();
        instancedMesh.setMatrixAt(i, dummy.matrix);
      }
      
      scene.add(instancedMesh);
      neurons.push({ 
        instancedMesh, 
        regionIdx, 
        count: region.count,
        baseColor: region.color 
      });
    });

    neuronsRef.current = neurons;
    setIsLoaded(true);

    // Animation loop
    let animationId;
    const animate = () => {
      animationId = requestAnimationFrame(animate);
      
      // Rotate scene slowly
      scene.rotation.y += 0.001;
      
      renderer.render(scene, camera);
    };
    animate();

    // Handle window resize
    const handleResize = () => {
      if (!canvasRef.current) return;
      const width = canvasRef.current.clientWidth;
      const height = canvasRef.current.clientHeight;
      camera.aspect = width / height;
      camera.updateProjectionMatrix();
      renderer.setSize(width, height);
    };
    window.addEventListener('resize', handleResize);

    // Cleanup
    return () => {
      cancelAnimationFrame(animationId);
      window.removeEventListener('resize', handleResize);
      renderer.dispose();
    };
  }, []);

  // Update neuron colors based on brain state
  useEffect(() => {
    if (!isLoaded || !brainState) return;

    const activities = [
      brainState.language_encoder_activity || [],
      brainState.language_decoder_activity || [],
      brainState.memory_center_activity || [],
      brainState.cognitive_center_activity || []
    ];

    neuronsRef.current.forEach((region) => {
      const activity = activities[region.regionIdx];
      if (!activity) return;

      for (let i = 0; i < region.count; i++) {
        const activation = activity[i] || 0;
        
        let color;
        if (activation < 0.5) {
          const t = activation * 2;
          color = new THREE.Color().setRGB(t, t, 1 - t);
        } else {
          const t = (activation - 0.5) * 2;
          color = new THREE.Color().setRGB(1, 1 - t, 0);
        }

        region.instancedMesh.setColorAt(i, color);
      }
      region.instancedMesh.instanceColor.needsUpdate = true;
    });
  }, [brainState, isLoaded]);

  return (
    <div className="neuron-visualizer" style={{ width: '100%', height: '500px', position: 'relative' }}>
      <canvas 
        ref={canvasRef} 
        style={{ width: '100%', height: '100%', display: 'block' }}
      />
      {!isLoaded && (
        <div style={{ 
          position: 'absolute', 
          top: '50%', 
          left: '50%', 
          transform: 'translate(-50%, -50%)',
          color: '#fff'
        }}>
          Loading 3D Visualization...
        </div>
      )}
    </div>
  );
}
