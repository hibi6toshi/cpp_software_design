# デザインパターンはどこにもある

```C++
#include <array>
#include <cstddef>
#include <cstdlib>
#include <memory_resource>
#include <string>
#include <vector>

int main() {
  std::array<std::byte, 1000> raw; // -① Note: not initialized!(未初期化)

  std::pmr::monotonic_buffer_resource buffer{ raw.data(), raw.size(), std::pmr::null_memory_resource()}; // ②

  std::pmr::vector<std::pmr::string> strings{ &buffer }; // ③

  strings.emplace_back("Srging longer than what SSO can handle");
  strings.emplace_back("Another long string that goes beyond SSO");
  strings.emplace_back("A third long string that cannot be gandled by SSO");

  // ...
  
  return EXIT_SUCCESS;
}
```
上例は、`std::pmr`名前空間（polymorphic_memory_resource）のアロケータを使う例です。
アロケータに`std::pmr::monotonic_buffer_resource`を用いています。全てのメモリ割り当てを確保済みのバイトバッファへ振り替えます。はじめに`std::array`により1000バイトのバッファを用意し①、`std::pmr::monotonic_buffer_resource`が使用するメモリ領域として先頭要素を指すポインタと、バッファサイズを渡します（`raw.data()`と`raw.size()`）②。
`monotonic_buffer_resource`の第3引数にはバックアップアロケータを与えます。バックアップアロケータは、バッファが枯渇した場合に使用されます。上例では追加メモリは不要なため、`std::pmr::null_memory_resource()`が返す、常にメモリ割り当てに失敗する標準アロケータのポインタを与えます。すなわち、好きなだけメモリを要求できますが、使い切れば、`std::null_memory_resource()`が指すアロケータが例外をスローします。
確保したバッファをアロケータとして`strings vector`へ渡し、全てのメモリ割り当てはこのバイトバッファから振り替えられるようにします③。 `vector`はアロケータを要素へ転送するため、要素が使用するメモリもバイトバッファから割り当てられます。emplace_back()で加えた3うtの文字列はいずれも長さが十分にあり、**短い文字列の最適化**(SSO, Small String Optimization)の対象になりませんが、上例では動的メモリは一切使用しておらず、全てバッファから割り当てられます。

一見すると、上例にデザインパターンは不要に見えます。しかし、このアロケータは少なくとも4つのデザインパターンを使用しています。Template Methodパターン、Decoratorパターン、Adapterパターン、Strategyパターンです。Singletonパターンも含めて数えれば、全部で5つのデザインパターンを使用しています。(②)
`null_memory_resource()`は静的ストレージ有効期限オブジェクト(`static storage duration ohject`)を指すポインタを返しますが、このオブジェクトが、アロケータインスタンスが一意であることを保証します。

`null_memory_resource()`, `monotonic_buffer_resource`もそうですが、pmr名前空間のアロケータは全て、`std::pmr::memory_resource`の基底クラスの派生です。次に挙げる`memory_resource`クラス定義から、最初のデザインパターンの存在がえるようになるでしょう。

```C++
#include <cstddef>

namespace std::pmr {
  class memory_resource {
    public:
      // ... a virtual destructor, some constructors and assignment operators

    [[nodiscard]] void* allocate(size_t bytes, size_t alignment);
    void deallocate(void* p, size_t bytes, size_t alignment);
    bool is_equal(memory_resource const& other) const noexcept;
  
  private:
    virtual void* do_allocate(size_t bytes, size_t alignment) = 0;
    virtual void do_deallocate(void* p, size_t bytes, size_t alignment) = 0;
    virtual bool do_is_equal(memory_resource const& other) const noexcept = 0;
  };
} // namespace std::pmr
// 仮想デストラクタ、コンストラクタ、代入オペレータなど
```

上例のクラスが定義する3つのpublic関数には、それぞれ対になるprivate仮想関数があります。publicのallocate(), deallocate(), is_equal()はクラス使用者向けのインタフェースであり、do_allocate(), do_deallocate(), do_is_equal()は派生クラス用のインターフェースです。この関心の分離はNVI（非仮想インタフェース、Non-Virtual Interface）イディオムの一例であり、これ自体がTemplate Methodパターンです。

上例が暗黙のうちに使用している、デザインパターンの2例目はDecoratorパターンです。Decoratorパターンは複数のアロケータを階層化し、アロケータの機能を拡張します。このアプローチは次の部分を見ると明らかです。

```C++
std::pmr::monotonic_buffer_resource buffer{ raw.data(), raw.size(), std::pmr::null_memory_resource()}; 
```

`null_memory_resource()`が返したアロケータを`monotonic_buffer_resource`へ渡し、機能を修飾します。`allocate()`から`monotonic_buffer_resource`にメモリ割り当てを要求すると、最終的にバックアップアロケータがコールされます。この方式により、複数の異なる種類のアロケータを実装でき、メモリ割り当て方式を複数備えるメモリサブシステムの構築が容易になります。機能を再利生し繋ぎ合わせる点が、Decoratorパターンの特徴です。

`std::string`は`std::basic_string<char>`の単なるエイリアスです。この点を踏まえれば、pmr名前空間の2つの型も、実は単なる型エイリアスだったと分かっても驚きではないでしょう。
```C++
namespace std::pmr {
  template<class CharT, class Traits = std::char_traits<CharT>>
  using basic_string = std::basic_string<CharT, Traits, std::pmr::polymorphic_allocator<CharT>>;

  template<class T>
  using vector = std::vector<T, std::pmr::polymorphic_allocator<T>>;
} // namespace std::pmr
```

上例の型エイリアスは、やはり標準のstd::vectorとstd::basic_stringクラスを指しますがアロケータのテンプレート引数を公開せず、代わりに`std::pmr::polymorphic_allocator`をアロケータとしています。これはAdaptorパターンの例です。Adaptorパターンの目的は元来合致しないインターフェースを繋げることにあります。
上例では、`polymorphic_allocator`が、古典的なC++アロケータの従来型静的インタフェースと、`std::pmr::memory_resource`の新しい動的アロケータインタフェースを繋ぐ役割をします。

アロケータのテンプレート引数を公開すれば、`std::vector`や`std::string`などの標準ライブラリのコンテナは、外部からメモリ割り当てをカスタマイズできるようになります。これはStrategyパターンの静的な例ですl。アルゴリズムをカスタマイズした場合と同じ目的を持ちます。

この例はデザインパターンが過去の遺物と呼ぶにはかけ離れた存在であることを示しています。
デザインパターンはどこでも使われており、抽象化、分離、柔軟性と拡張性の向上、どちらもデザインパターンを基にしている場合がほとんどです。

## まとめ
- なんらかの抽象化やソフトウェアエンティティの分離は、多くの場合既存のデザインパターンの現れであることを理解する。
- さまざまなデザインパターンを学び、その分離する目的を理解する。
- 必要に応じて、目的に即したデザインパターンを使用する。
