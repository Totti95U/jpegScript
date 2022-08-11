# jpegScript
![top image](https://github.com/Totti95U/jpegScript/blob/image/main.png)
jpeg に圧縮したときに発生するノイズを再現する AviUtl のスクリプトです。
ダウンロードは[コチラ](https://github.com/Totti95U/jpegScript/releases)

機能説明
オブジェクトを jpeg と同等の方式で変換し、その逆変換を施すことで、
jpeg で保存した時のようなノイズを発生させます。(YCbCr 空間でダウンサンプリングを行いますがその時の比率は 1:2:2 です)

重めの処理なのでリサイズでオブジェクトのピクセル数を少なく(目安は 500 x 500 px 以下) してから
エフェクトを施すことを推奨します。(さもないと応答しなくなります。)

各スライダーの機能の説明
・品　　　　質：圧縮の際の非可逆度合いを指定します。0 に近いほど jpeg 特有のノイズが多くなり、100 に近いほど元通りになります。
![quality image](https://github.com/Totti95U/jpegScript/blob/image/quality.png)

・インタレース：画像の高周波成分を何処まで使うかを指定します。64 で全ての周波数成分を使い、1 に近づくほど低周波成分しか使用しなくなります。
![interlace image](https://github.com/Totti95U/jpegScript/blob/image/interlace.png)