# オーバーロードセットの意味的要件を理解する

## フリー関数の威力: コンパイル時抽象化
コンセプトに続く、2つ目のコンパイル時抽象化方法は、フリー関数を用いた関数オーバーロードです。同じ名前を持つ関数が複数ある場合（オーバーロードセット）、与えられた引数を基にコンパイラが時際にコールする関数を決定します。
フリー関数はどんな型にも追加できます。intやstd::stringなど、既存コードに干渉せず、任意の型に追加可能です。
（メンバ関数は追加できない場合があります。メンバ関数の追加は既存コードの変更を伴い、メンバ関数を持てない型や、開発者がコードを変更できない型などには追加できません。）
フリー関数ならば、開放/閉鎖原則に完全に従えます。単純にコードを追加するだけで機能を追加でき、既存コードを修正する必要はありません。

```C++
template <tyupename Range>
void traverseRange(Range const& range) {
  for(auto pos = range.begin(); pos != range.end(); ++pos) {
    // ...
  }
}
```
上例のtraverseRange()関数は、イテレータを用いた繰り返し処理で、多数のコンテナで機能しますが、組み込み配列には使えません。
配列にはbegin()とend()メンバ関数が存在しないためです。
対策の1つはstd::arrayを使うことです。

ここではtraverseRange()関数の設計の観点を失わないっようにします。
traverseRange()はメンバカンスのbegin(),end()に依存しており、自らを制限してしまっています。Range型に対し、begin()、end()メンバ関数対応という人工的な要件を設け、自らの適用性を制限しているのです。しかしこの関数の適用性を著しく向上させる簡単な方法があり、それによって解決できます。
フリー関数のbegin()とend()のオーバーロードセットを作成すれば良いのです。
  
```C++
template<typename Range>
void traverseRange(Range const& range) {
  using std::begin; // using declarations for the purpose of calling `begin()` and `end()
  using std::end;   // unqualified to enable ADL
                    // ADLを有効にするため、修飾しないbegin()/end()をコールするようusing宣言する

  for(auto pos=begin(range); pos!=end(range); ++pos) {
    // ...
  }
}
```

上例の関数は処理内容こそ先にあげたものと変わりませんが、人工的な要件を付加しておらず、自らを制限することがありません。
制約は一切なく、**どんな型でも**フリー関数begin()とend()を持てますし、現在持っていなくても追加実装すればすみます。しかも、既存コードを変更する必要はありません。
上例の関数は、どんな種類のRangeに対しても動作し、要件を満たさない型でも修正せず、使用できます。

フリー関数は関心を分離し、単一責任にも従った、非常に綺麗な解です。クラス外で処理を実装すれば、クラスから処理への依存関係を自動的に削減できます。メンバ関数は暗黙的に先頭引数thisポインタを持ちますが、フリー関数はこれを持たないからです。同時に関数は独立的に動作するため、他のクラスからも利用可能となります。

## フリー関数の問題点: 動作に対する期待
フリー関数が威力を発揮できるのは、オーバーロード関数が一連の規則に従い、期待する動作を満たす場合に限られます。すなわち、LSPに従う場合にのみ有効に機能するのです。
読者がWidget型を書き上げ、これから専用のswap()処理を加える場合を考えてみます。

```C++
// ---- Widget.h ----
struct Widget {
  int i;
  int j;
};

void swap(Widget& w1, Widget&w2) {
  using std::swap;
  swap(w1.i, w2.i);
}
```
上例のWidgetはint i, j をラッピングしただけのものです。ここでWidgetに付随するフリー関数として、swap()関数を加えました。しかし、その実装はiのみを交換し、jを交換していません。

上例の実装では、swap()関数は期待される内容を満たしていません。オブジェクトの外部から見える状態全てを交換対象とするswap()関数が期待されているのです。
オーバーロードセットへこの関数を追加する以上、期待される動作を満たすとされるのが当然です。LSPに従いなさいということです。

まとめると、関数オーバーロードは強力なコンパイル時抽象化機構です。特にジェネリックプログラミングではその威力を遺憾なく発揮します。しかし、軽々しく扱ってもいけません。基底クラスやコンセプト同様に、オーバーロードセットも意味的な要件を表現するため、LSPに従う必要があります。そうしないと正しく動作しないでしょう。

## まとめ
- 関数オーバーロードとは、コンパイル時抽象化機構と認識する。
- オーバーロードセット内の関数の動作には、期待される内容があることを忘れてはいけない。
- 既存の名前や命名規則に注意する。