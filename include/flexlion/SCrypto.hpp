#pragma once
#define FLEXLION_OBF_KEY Flexlion::generate_key(Flexlion::RandomSeed())
namespace Flexlion
{
	using size_type = unsigned long long;
	using key_type = unsigned long long;
	constexpr int RandomSeed(void) {
        return '0'      * -40271 +
            __TIME__[7] *      1 +
            __TIME__[6] *     10 +
            __TIME__[4] *     60 +
            __TIME__[3] *    600 +
            __TIME__[1] *   3600 +
            __TIME__[0] *  36000;
    };
	constexpr key_type generate_key(key_type seed)
	{
		key_type key = seed;
		key ^= (key >> 33);
		key *= 0xff51afd7ed558ccd;
		key ^= (key >> 33);
		key *= 0xc4ceb9fe1a85ec53;
		key ^= (key >> 33);
		key |= 0x0101010101010101ull;

		return key;
	}
	constexpr void cipher(char* data, size_type size, key_type key)
	{
		for (size_type i = 0; i < size; i++)
		{
			data[i] ^= char(key >> ((i % 8) * 8));
		}
	}
	template <size_type N, key_type KEY>
	class obfuscator
	{
	public:
		constexpr obfuscator(const char* data)
		{
			for (size_type i = 0; i < N; i++)
			{
				m_data[i] = data[i];
			}
			cipher(m_data, N, KEY);
		}
		constexpr const char* data() const
		{
			return &m_data[0];
		}
		constexpr size_type size() const
		{
			return N;
		}
		constexpr key_type key() const
		{
			return KEY;
		}
	private:
		char m_data[N]{};
	};
	template <size_type N, key_type KEY>
	class obfuscated_data
	{
	public:
		obfuscated_data(const obfuscator<N, KEY>& obfuscator)
		{
			// Copy obfuscated data
			for (size_type i = 0; i < N; i++)
			{
				m_data[i] = obfuscator.data()[i];
			}
		}
		~obfuscated_data()
		{
			for (size_type i = 0; i < N; i++)
			{
				m_data[i] = 0;
			}
		}
		operator char*()
		{
			decrypt();
			return m_data;
		}
		void decrypt()
		{
			if (m_encrypted)
			{
				cipher(m_data, N, KEY);
				m_encrypted = false;
			}
		}
		void encrypt()
		{
			if (!m_encrypted)
			{
				cipher(m_data, N, KEY);
				m_encrypted = true;
			}
		}
		bool is_encrypted() const
		{
			return m_encrypted;
		}
	private:
		char m_data[N];
		bool m_encrypted{ true };
	};
	template <size_type N, key_type KEY = FLEXLION_OBF_KEY>
	constexpr auto make_obfuscator(const char(&data)[N])
	{
		return obfuscator<N, KEY>(data);
	}
};

#define OBF_CHAR(data) static_cast<char*>(OBFUSCATE(data))

#define OBFUSCATE(data) STR_OBFUSCATE_KEY(data, FLEXLION_OBF_KEY)

#define STR_OBFUSCATE_KEY(data, key) \
	[]() -> Flexlion::obfuscated_data<sizeof(data)/sizeof(data[0]), key>& { \
		constexpr auto n = sizeof(data)/sizeof(data[0]); \
		constexpr auto obfuscator = Flexlion::make_obfuscator<n, key>(data); \
		static auto obfuscated_data = Flexlion::obfuscated_data<n, key>(obfuscator); \
		return obfuscated_data; \
	}()

