
/* thx ChatGPT providing corresponding structures : c_set(avl-tree based), c_map(avl-tree based)
(btw avl_tree were implemented before but I lost source code of it)*/

template<typename T>
struct Optional
{
    bool has_value = false;
    T value;

    Optional( ) : has_value( false ) { }
    Optional( const T& v ) : has_value( true ), value( v ) { }

    operator bool( ) const { return has_value; }
    //T& operator*( ) { return value; }
    T& get( ) { return value; }
    //const T& operator*( ) const { return value; }
};

template<typename T, long long int MaxSize>
class c_set
{

    struct Node
    {
        T key;
        long long int height;
        long long int left = -1;
        long long int right = -1;
    };

    Node nodes[ MaxSize ];
    long long int freeList[ MaxSize ];
    long long int freeTop = 0;
    long long int root = -1;
    long long int size_ = 0;

    long long int allocateNode( const T& key )
    {
        if ( freeTop == 0 )
            throw "c_set: Out of memory";

        long long int idx = freeList[ --freeTop ];
        nodes[ idx ].key = key;
        nodes[ idx ].height = 1;
        nodes[ idx ].left = -1;
        nodes[ idx ].right = -1;
        return idx;
    }

    void freeNode( long long int idx )
    {
        freeList[ freeTop++ ] = idx;
    }

    long long int height( long long int n ) const
    {
        return ( n == -1 ) ? 0 : nodes[ n ].height;
    }

    void updateHeight( long long int n )
    {
        nodes[ n ].height = 1 + ( ( height( nodes[ n ].left ) > height( nodes[ n ].right ) ) ? height( nodes[ n ].left ) : height( nodes[ n ].right ) );
    }

    long long int balanceFactor( long long int n ) const
    {
        return height( nodes[ n ].left ) - height( nodes[ n ].right );
    }

    long long int rotateRight( long long int y )
    {
        long long int x = nodes[ y ].left;
        long long int T2 = nodes[ x ].right;

        nodes[ x ].right = y;
        nodes[ y ].left = T2;

        updateHeight( y );
        updateHeight( x );

        return x;
    }

    long long int rotateLeft( long long int x )
    {
        long long int y = nodes[ x ].right;
        long long int T2 = nodes[ y ].left;

        nodes[ y ].left = x;
        nodes[ x ].right = T2;

        updateHeight( x );
        updateHeight( y );

        return y;
    }

    long long int balance( long long int n )
    {
        updateHeight( n );
        long long int bf = balanceFactor( n );

        if ( bf > 1 )
        {
            if ( balanceFactor( nodes[ n ].left ) < 0 )
                nodes[ n ].left = rotateLeft( nodes[ n ].left );
            return rotateRight( n );
        }

        if ( bf < -1 )
        {
            if ( balanceFactor( nodes[ n ].right ) > 0 )
                nodes[ n ].right = rotateRight( nodes[ n ].right );
            return rotateLeft( n );
        }

        return n;
    }

    long long int insert( long long int node, const T& key, bool& inserted )
    {
        if ( node == -1 )
        {
            inserted = true;
            return allocateNode( key );
        }

        if ( key == nodes[ node ].key )
        {
            inserted = false;
            return node;
        }

        if ( key < nodes[ node ].key )
            nodes[ node ].left = insert( nodes[ node ].left, key, inserted );
        else
            nodes[ node ].right = insert( nodes[ node ].right, key, inserted );

        return balance( node );
    }

    bool contains( long long int node, const T& key ) const
    {
        if ( node == -1 ) return false;
        if ( key == nodes[ node ].key ) return true;
        if ( key < nodes[ node ].key )
            return contains( nodes[ node ].left, key );
        return contains( nodes[ node ].right, key );
    }

    Optional<T> lower_bound( long long int node, const T& key ) const
    {
        long long int curr = node;
        Optional<T> res;

        while ( curr != -1 )
        {
            auto l = nodes[ curr ].key;
            if ( nodes[ curr ].key >= key )
            {
                // Сохраняем текущий узел, так как его ключ >= key
                res = Optional<T>( nodes[ curr ].key );
                // Переходим в левое поддерево, чтобы искать минимальный элемент >= key
                curr = nodes[ curr ].left;
            }
            else
            {
                // Переходим в правое поддерево, так как ключ текущего узла < key
                curr = nodes[ curr ].right;
            }
        }

        return res;
    }

    long long int erase( long long int node, const T& key, bool& erased )
    {
        if ( node == -1 )
        {
            erased = false;  // Элемент не найден
            return node;
        }

        if ( key < nodes[ node ].key )
        {
            nodes[ node ].left = erase( nodes[ node ].left, key, erased );
        }
        else if ( key > nodes[ node ].key )
        {
            nodes[ node ].right = erase( nodes[ node ].right, key, erased );
        }
        else
        {
            erased = true;

            // Если у узла нет детей или один из них отсутствует
            if ( nodes[ node ].left == -1 || nodes[ node ].right == -1 )
            {
                long long int child = ( nodes[ node ].left != -1 ) ? nodes[ node ].left : nodes[ node ].right;
                freeNode( node );  // Освобождаем этот узел
                return child;      // Возвращаем индексацию ребенка (или -1, если его нет)
            }

            // Если у узла два ребенка
            long long int minNode = nodes[ node ].right;
            while ( nodes[ minNode ].left != -1 )
                minNode = nodes[ minNode ].left;

            // Переносим минимальный элемент из правого поддерева в текущий узел
            nodes[ node ].key = nodes[ minNode ].key;

            // Удаляем минимальный узел из правого поддерева
            nodes[ node ].right = erase( nodes[ node ].right, nodes[ minNode ].key, erased );
        }

        return balance( node );
    }

public:
    c_set( )
    {
        for ( long long int i = 0; i < MaxSize; ++i )
            freeList[ i ] = MaxSize - 1 - i;
        freeTop = MaxSize;
    }

    c_set( const T& o ) : c_set( )
    {
        this->insert( o );
    }

    bool insert( const T& key )
    {
        bool inserted = false;
        root = insert( root, key, inserted );
        if ( inserted ) ++size_;
        return inserted;
    }

    bool erase( const T& key )
    {
        bool erased = false;
        root = erase( root, key, erased );
        if ( erased ) --size_;
        return erased;
    }

    bool contains( const T& key ) const
    {
        return contains( root, key );
    }

    Optional<T> lower_bound( const T& key ) const
    {
        return lower_bound( root, key );
    }

    long long int size( ) const { return size_; }
};

template<typename Key, typename Value, long long int MaxSize>
class c_map
{

    long long int erase( long long int node, const Key& key, bool& erased )
    {
        if ( node == -1 )
            return -1;

        if ( key < nodes[ node ].key )
        {
            nodes[ node ].left = erase( nodes[ node ].left, key, erased );
        }
        else if ( key > nodes[ node ].key )
        {
            nodes[ node ].right = erase( nodes[ node ].right, key, erased );
        }
        else
        {
            erased = true;

            if ( nodes[ node ].left == -1 || nodes[ node ].right == -1 )
            {
                long long int child = ( nodes[ node ].left != -1 ) ? nodes[ node ].left : nodes[ node ].right;
                freeList[ freeTop++ ] = node;
                return child;
            }

            // Узел с двумя детьми: найти минимальный в правом поддереве
            long long int minNode = nodes[ node ].right;
            while ( nodes[ minNode ].left != -1 )
                minNode = nodes[ minNode ].left;

            nodes[ node ].key = nodes[ minNode ].key;
            nodes[ node ].value = nodes[ minNode ].value;

            nodes[ node ].right = erase( nodes[ node ].right, nodes[ minNode ].key, erased );
        }

        return balance( node );
    }

    struct Node
    {
        Key key;
        Value value;
        long long int height;
        long long int left = -1;
        long long int right = -1;
    };

    Node nodes[ MaxSize ];
    long long int freeList[ MaxSize ];
    long long int freeTop = 0;
    long long int root = -1;
    long long int size_ = 0;

    long long int allocateNode( const Key& key, const Value& value )
    {
        if ( freeTop == 0 )
            throw "c_map: Out of memory";

        long long int idx = freeList[ --freeTop ];
        nodes[ idx ].key = key;
        nodes[ idx ].value = value;
        nodes[ idx ].height = 1;
        nodes[ idx ].left = -1;
        nodes[ idx ].right = -1;
        return idx;
    }

    long long int height( long long int n ) const
    {
        return ( n == -1 ) ? 0 : nodes[ n ].height;
    }

    void updateHeight( long long int n )
    {
        long long int lh = height( nodes[ n ].left );
        long long int rh = height( nodes[ n ].right );
        nodes[ n ].height = 1 + ( lh > rh ? lh : rh );
    }

    long long int balanceFactor( long long int n ) const
    {
        return height( nodes[ n ].left ) - height( nodes[ n ].right );
    }

    long long int rotateRight( long long int y )
    {
        long long int x = nodes[ y ].left;
        long long int T2 = nodes[ x ].right;

        nodes[ x ].right = y;
        nodes[ y ].left = T2;

        updateHeight( y );
        updateHeight( x );
        return x;
    }

    long long int rotateLeft( long long int x )
    {
        long long int y = nodes[ x ].right;
        long long int T2 = nodes[ y ].left;

        nodes[ y ].left = x;
        nodes[ x ].right = T2;

        updateHeight( x );
        updateHeight( y );
        return y;
    }

    long long int balance( long long int n )
    {
        updateHeight( n );
        long long int bf = balanceFactor( n );

        if ( bf > 1 )
        {
            if ( balanceFactor( nodes[ n ].left ) < 0 )
                nodes[ n ].left = rotateLeft( nodes[ n ].left );
            return rotateRight( n );
        }

        if ( bf < -1 )
        {
            if ( balanceFactor( nodes[ n ].right ) > 0 )
                nodes[ n ].right = rotateRight( nodes[ n ].right );
            return rotateLeft( n );
        }

        return n;
    }

    long long int insert( long long int node, const Key& key, const Value& value, bool& inserted )
    {
        if ( node == -1 )
        {
            inserted = true;
            return allocateNode( key, value );
        }

        if ( key == nodes[ node ].key )
        {
            inserted = false;
            nodes[ node ].value = value; // заменяем значение
            return node;
        }

        if ( key < nodes[ node ].key )
            nodes[ node ].left = insert( nodes[ node ].left, key, value, inserted );
        else
            nodes[ node ].right = insert( nodes[ node ].right, key, value, inserted );

        return balance( node );
    }

    Value* find( long long int node, const Key& key )
    {
        if ( node == -1 ) return nullptr;
        if ( key == nodes[ node ].key ) return &nodes[ node ].value;
        if ( key < nodes[ node ].key )
            return find( nodes[ node ].left, key );
        return find( nodes[ node ].right, key );
    }

    const Value* find( long long int node, const Key& key ) const
    {
        if ( node == -1 ) return nullptr;
        if ( key == nodes[ node ].key ) return &nodes[ node ].value;
        if ( key < nodes[ node ].key )
            return find( nodes[ node ].left, key );
        return find( nodes[ node ].right, key );
    }

public:
    c_map( )
    {
        for ( long long int i = 0; i < MaxSize; ++i )
            freeList[ i ] = MaxSize - 1 - i;
        freeTop = MaxSize;
    }

    Value* find( const Key& key )
    {
        return find( root, key );
    }

    const Value* find( const Key& key ) const
    {
        return find( root, key );
    }

    bool insert( const Key& key )
    {
        bool inserted = false;
        root = insert( root, key, inserted );
        if ( inserted ) ++size_;
        return inserted;
    }

    Value& operator[]( const Key& key )
    {
        Value* existing = find( root, key );
        if ( existing )
            return *existing;

        bool inserted = false;
        root = insert( root, key, Value( ), inserted );
        if ( inserted ) ++size_;

        Value* result = find( root, key );
        if ( !result )
            throw "c_map: long long internal error on operator[]";

        return *result;
    }

    Value* get( const Key& key )
    {
        return find( root, key );
    }

    const Value* get( const Key& key ) const
    {
        return find( root, key );
    }

    bool erase( const Key& key )
    {
        bool erased = false;
        root = erase( root, key, erased );
        if ( erased )
            --size_;
        return erased;
    }

    long long int size( ) const { return size_; }
};
