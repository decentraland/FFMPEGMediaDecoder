using HTC.UnityPlugin.Multimedia;
using UnityEngine;

public class FFMPEGPlayer : MonoBehaviour
{
    public string videoPath = "";

    private FFMPEGDecoder decoder;
    private Texture2D localTexture;

    private void Awake()
    {
        FFMPEGDecoderWrapper.nativeCleanAll();
        decoder = new FFMPEGDecoder(videoPath);
    }

    private void Update()
    {
        if (decoder.getDecoderState() == FFMPEGDecoder.DecoderState.INITIALIZED)
        {
            var material = GetComponent<MeshRenderer>().sharedMaterial;
            var texture = decoder.GetTexture();
            if (texture != null && localTexture == null)
            {
                material.mainTexture = texture;
                localTexture = texture;
                decoder.Play();
            }
        }
        else if (decoder.getDecoderState() == FFMPEGDecoder.DecoderState.START)
        {
            decoder.UpdateVideoTexture();
        }
    }
}