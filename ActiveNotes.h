// https://arduino.stackexchange.com/questions/69064/use-stdlist-in-arduino-programming
#define maxLength (16)
class Notes { 
public:
    byte length;
    byte pitches[maxLength];
    byte playerIndices[maxLength];
    void append(byte pitch, byte playerIndex) {
        if (length < maxLength) {
            pitches[length] = pitch;
            playerIndices[length] = playerIndex;
            length++;
        }
    }
    void remove(byte index) {
        if (index >= length) return;
        memmove(&pitches[index], &pitches[index+1], length - index - 1);
        memmove(&playerIndices[index], &playerIndices[index+1], length - index - 1);
        length--;
    }

    #if DEBUG
      byte lastLength;
      byte lastPitches[maxLength];
      bool pitchesChanged(){
        bool result = false;
        if (lastLength != length){
          result = true;
          lastLength = length;
          for (byte i = 0; i<length; i++){
            lastPitches[i] = pitches[i];
          }
        }
        else{
          for (byte i = 0; i<length; i++){
            result |= (lastPitches[i]!=pitches[i]);
            lastPitches[i] = pitches[i];
          }
        }
        return result;
      }
      void debug(){
        if(pitchesChanged()){
          Serial.print("Notes [");
          Serial.print(length);
          if(length>0){
              Serial.println("] (pitch, index):");
              for (byte i = 0; i < length; i++){
                  Serial.print(pitches[i]);
                  Serial.print(", ");
              }
              Serial.println("/");
              
              for (byte i = 0; i < length; i++){
                  Serial.print(playerIndices[i]);
                  Serial.print(", ");
              }
              Serial.println("/");
          }
          else{
              Serial.println("]");
          }
        }
      }
    #endif
};